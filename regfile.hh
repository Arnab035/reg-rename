#ifndef __CPU_REGFILE_HH__
#define __CPU_REGFILE_HH__


#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

#include "reg_class.hh"

namespace workflow
{

class RegFile
{
  private:
    std::vector<uint8_t> data;
    const size_t _size;
    const size_t _regShift;
    const size_t _regBytes;

  public:
    const RegClass &regClass;

    RegFile(const RegClass &info, const size_t new_size) :
        data(new_size << info.regShift()), _size(new_size),
        _regShift(info.regShift()), _regBytes(info.regBytes()),
        regClass(info)
    {}

    RegFile(const RegClass &info) : RegFile(info, info.numRegs()) {}

    size_t size() const { return _size; }
    size_t regShift() const { return _regShift; }
    size_t regBytes() const { return _regBytes; }

    template <typename Reg=RegVal>
    Reg &
    reg(size_t idx)
    {
        assert(sizeof(Reg) == _regBytes && idx < _size);
        return *reinterpret_cast<Reg *>(data.data() + (idx << _regShift));
    }
    template <typename Reg=RegVal>
    const Reg &
    reg(size_t idx) const
    {
        assert(sizeof(Reg) == _regBytes && idx < _size);
        return *reinterpret_cast<const Reg *>(
                data.data() + (idx << _regShift));
    }

    void *
    ptr(size_t idx)
    {
        return data.data() + (idx << _regShift);
    }

    const void *
    ptr(size_t idx) const
    {
        return data.data() + (idx << _regShift);
    }

    void
    get(size_t idx, void *val) const
    {
        std::memcpy(val, ptr(idx), _regBytes);
    }

    void
    set(size_t idx, const void *val)
    {
        std::memcpy(ptr(idx), val, _regBytes);
    }

    void clear() { std::fill(data.begin(), data.end(), 0); }
};
}

#endif
