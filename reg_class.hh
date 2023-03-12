#ifndef __CPU__REG_CLASS_HH__
#define __CPU__REG_CLASS_HH__

#include <cstddef>
#include <iterator>
#include <string>
#include <cassert>

#include "debug.hh"


namespace workflow 
{

/** Enumerate the classes of registers.
 *  This gem5 enum can possibly be extended.
 */
enum RegClassType
{
    CapRegClass,       ///< CapabilityRegisters
    InvalidRegClass = -1
};

// "Standard" register class names. Using these is encouraged but optional.
inline constexpr char CapRegClassName[] = "capability";

using RegIndex = uint16_t;
using RegVal = uint64_t;

class RegClass;
class RegClassIterator;
class BaseISA;

/**
 * @ingroup api_base_utils
 */
template <class T>
static constexpr std::enable_if_t<std::is_integral_v<T>, int>
floorLog2(T x)
{
    assert(x > 0);

    // A guaranteed unsigned version of x.
    uint64_t ux = (typename std::make_unsigned<T>::type)x;

    int y = 0;
    constexpr auto ts = sizeof(T);

    if (ts >= 8 && (ux & 0xffffffff00000000ULL)) { y += 32; ux >>= 32; }
    if (ts >= 4 && (ux & 0x00000000ffff0000ULL)) { y += 16; ux >>= 16; }
    if (ts >= 2 && (ux & 0x000000000000ff00ULL)) { y +=  8; ux >>=  8; }
    if (ux & 0x00000000000000f0ULL) { y +=  4; ux >>=  4; }
    if (ux & 0x000000000000000cULL) { y +=  2; ux >>=  2; }
    if (ux & 0x0000000000000002ULL) { y +=  1; }

    return y;
}

/**
 * @ingroup api_base_utils
 */
template <class T>
static constexpr int
ceilLog2(const T& n)
{
    assert(n > 0);
    if (n == 1)
        return 0;

    return floorLog2(n - (T)1) + 1;
}

/**
 * Register ID: describe an architectural register with its class and index.
 * This structure is used instead of just the register index to disambiguate
 * between different classes of registers. For example, a integer register with
 * index 3 is represented by Regid(IntRegClass, 3).
 */

class RegId
{
  protected:
    const RegClass *_regClass = nullptr;
    RegIndex regIdx;
    int numPinnedWrites;

    friend struct std::hash<RegId>;
    friend class RegClassIterator;

  public:
    inline constexpr RegId();

    constexpr RegId(const RegClass &reg_class, RegIndex reg_idx)
        : _regClass(&reg_class), regIdx(reg_idx), numPinnedWrites(0)
    {}

    constexpr operator RegIndex() const
    {
        return index();
    }

    constexpr bool
    operator==(const RegId& that) const
    {
        return classValue() == that.classValue() && regIdx == that.index();
    }

    constexpr bool
    operator!=(const RegId& that) const
    {
        return !(*this==that);
    }

    /** Order operator.
     * The order is required to implement maps with key type RegId
     */
    constexpr bool
    operator<(const RegId& that) const
    {
        return classValue() < that.classValue() ||
            (classValue() == that.classValue() && (regIdx < that.index()));
    }

    /**
     * Return true if this register can be renamed
     */
    constexpr bool
    isRenameable() const
    {
        return classValue() != InvalidRegClass;
    }

    /** @return true if it is of the specified class. */
    inline constexpr bool is(RegClassType reg_class) const;

    /** Index accessors */
    /** @{ */
    constexpr RegIndex index() const { return regIdx; }

    /** Class accessor */
    constexpr const RegClass &regClass() const { return *_regClass; }
    inline constexpr RegClassType classValue() const;
    /** Return a const char* with the register class name. */
    inline constexpr const char* className() const;

    inline constexpr bool isFlat() const;
    inline RegId flatten(const BaseISA &isa) const;

    int getNumPinnedWrites() const { return numPinnedWrites; }
    void setNumPinnedWrites(int num_writes) { numPinnedWrites = num_writes; }

    friend inline std::ostream& operator<<(std::ostream& os, const RegId& rid);
};

class RegClassOps
{
  public:
    /** Print the name of the register specified in id. */
    virtual std::string regName(const RegId &id) const;
    /** Print the value of a register pointed to by val of size size. */
    virtual std::string valString(const void *val, size_t size) const;
    /** Flatten register id id using information in the ISA object isa. */
    virtual RegId
    flatten(const BaseISA &isa, const RegId &id) const
    {
        return id;
    }
};

class RegClassIterator;

class RegClass
{
  private:
    RegClassType _type;
    const char *_name;

    size_t _numRegs;
    size_t _regBytes = sizeof(RegVal);
    // This is how much to shift an index by to get an offset of a register in
    // a register file from the register index, which would otherwise need to
    // be calculated with a multiply.
    size_t _regShift = ceilLog2(sizeof(RegVal));

    static inline RegClassOps defaultOps;
    const RegClassOps *_ops = &defaultOps;
    const debug::Flag &debugFlag;

    bool _flat = true;

  public:
    constexpr RegClass(RegClassType type, const char *new_name,
            size_t num_regs, const debug::Flag &debug_flag) :
        _type(type), _name(new_name), _numRegs(num_regs), debugFlag(debug_flag)
    {}

    constexpr RegClass
    needsFlattening() const
    {
        RegClass reg_class = *this;
        reg_class._flat = false;
        return reg_class;
    }

    constexpr RegClass
    ops(const RegClassOps &new_ops) const
    {
        RegClass reg_class = *this;
        reg_class._ops = &new_ops;
        return reg_class;
    }

    template <class RegType>
    constexpr RegClass
    regType() const
    {
        RegClass reg_class = *this;
        reg_class._regBytes = sizeof(RegType);
        reg_class._regShift = ceilLog2(reg_class._regBytes);
        return reg_class;
    }

    constexpr RegClassType type() const { return _type; }
    constexpr const char *name() const { return _name; }
    constexpr size_t numRegs() const { return _numRegs; }
    constexpr size_t regBytes() const { return _regBytes; }
    constexpr size_t regShift() const { return _regShift; }
    constexpr const debug::Flag &debug() const { return debugFlag; }
    constexpr bool isFlat() const { return _flat; }

    std::string regName(const RegId &id) const { return _ops->regName(id); }
    std::string
    valString(const void *val) const
    {
        return _ops->valString(val, regBytes());
    }
    RegId
    flatten(const BaseISA &isa, const RegId &id) const
    {
        return isFlat() ? id : _ops->flatten(isa, id);
    }

    using iterator = RegClassIterator;

    inline iterator begin() const;
    inline iterator end() const;

    inline constexpr RegId operator[](RegIndex idx) const;
};

inline constexpr RegClass
    invalidRegClass(InvalidRegClass, "invalid", 0, debug::InvalidReg);

constexpr RegId::RegId() : RegId(invalidRegClass, 0) {}

constexpr bool
RegId::is(RegClassType reg_class) const
{
    return _regClass->type() == reg_class;
}

constexpr RegClassType RegId::classValue() const { return _regClass->type(); }
constexpr const char* RegId::className() const { return _regClass->name(); }

constexpr bool RegId::isFlat() const { return _regClass->isFlat(); }

RegId
RegId::flatten(const BaseISA &isa) const
{
    return _regClass->flatten(isa, *this);
}

std::ostream&
operator<<(std::ostream& os, const RegId& rid)
{
    return os << rid.regClass().regName(rid);
}

class RegClassIterator
{
  private:
    RegId id;

    RegClassIterator(const RegClass &reg_class, RegIndex idx) :
        id(reg_class, idx)
    {}

    friend class RegClass;

  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::size_t;
    using value_type = const RegId;
    using pointer = value_type *;
    using reference = value_type &;

    reference operator*() const { return id; }
    pointer operator->() { return &id; }

    RegClassIterator &
    operator++()
    {
        id.regIdx++;
        return *this;
    }

    RegClassIterator
    operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool
    operator==(const RegClassIterator &other) const
    {
        return id == other.id;
    }

    bool
    operator!=(const RegClassIterator &other) const
    {
        return id != other.id;
    }
};

RegClassIterator
RegClass::begin() const
{
    return RegClassIterator(*this, 0);
}

RegClassIterator
RegClass::end() const
{
    return RegClassIterator(*this, numRegs());
}

constexpr RegId
RegClass::operator[](RegIndex idx) const
{
    return RegId(*this, idx);
}

/** Physical register ID.
 * Like a register ID but physical. The inheritance is private because the
 * only relationship between this types is functional, and it is done to
 * prevent code replication. */
class PhysRegId : private RegId
{
  private:
    RegIndex flatIdx;
    int numPinnedWritesToComplete;
    bool pinned;

  public:
    explicit PhysRegId() : RegId(invalidRegClass, -1), flatIdx(-1),
                           numPinnedWritesToComplete(0)
    {}

    /** Scalar PhysRegId constructor. */
    explicit PhysRegId(const RegClass &reg_class, RegIndex _regIdx,
              RegIndex _flatIdx)
        : RegId(reg_class, _regIdx), flatIdx(_flatIdx),
          numPinnedWritesToComplete(0), pinned(false)
    {}

    /** Visible RegId methods */
    /** @{ */
    using RegId::index;
    using RegId::regClass;
    using RegId::classValue;
    using RegId::className;
    using RegId::is;
     /** @} */
    /**
     * Explicit forward methods, to prevent comparisons of PhysRegId with
     * RegIds.
     */
    /** @{ */
    bool
    operator<(const PhysRegId& that) const
    {
        return RegId::operator<(that);
    }

    bool
    operator==(const PhysRegId& that) const
    {
        return RegId::operator==(that);
    }

    bool
    operator!=(const PhysRegId& that) const
    {
        return RegId::operator!=(that);
    }
    /** @} */

    /**
     * Returns true if this register is always associated to the same
     * architectural register.
     */
    bool isFixedMapping() const { return !isRenameable(); }

    /** Flat index accessor */
    const RegIndex& flatIndex() const { return flatIdx; }

    int getNumPinnedWrites() const { return numPinnedWrites; }

    void
    setNumPinnedWrites(int numWrites)
    {
        // An instruction with a pinned destination reg can get
        // squashed. The numPinnedWrites counter may be zero when
        // the squash happens but we need to know if the dest reg
        // was pinned originally in order to reset counters properly
        // for a possible re-rename using the same physical reg (which
        // may be required in case of a mem access order violation).
        pinned = (numWrites != 0);
        numPinnedWrites = numWrites;
    }

    void decrNumPinnedWrites() { --numPinnedWrites; }
    void incrNumPinnedWrites() { ++numPinnedWrites; }

    bool isPinned() const { return pinned; }

    int
    getNumPinnedWritesToComplete() const
    {
        return numPinnedWritesToComplete;
    }

    void
    setNumPinnedWritesToComplete(int numWrites)
    {
        numPinnedWritesToComplete = numWrites;
    }

    void decrNumPinnedWritesToComplete() { --numPinnedWritesToComplete; }
    void incrNumPinnedWritesToComplete() { ++numPinnedWritesToComplete; }
};

using PhysRegIdPtr = PhysRegId*;
using RegIdPtr = RegId*;  // may need it in CAM

} //  namespace workflow
#endif //__CPU__REG_CLASS_HH__ 
