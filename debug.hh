#ifndef __BASE_DEBUG_HH__
#define __BASE_DEBUG_HH__

#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace debug
{

void breakpoint();

class Flag
{
  protected:
    static bool _globalEnable; // whether debug tracings are enabled

    bool _tracing = false; // tracing is enabled and flag is on

    const char *_name;
    const char *_desc;

    virtual void sync() { }

  public:
    Flag(const char *name, const char *desc);
    virtual ~Flag();

    std::string name() const { return _name; }
    std::string desc() const { return _desc; }

    bool tracing() const { return /*TRACING_ON &&*/ _tracing; }

    virtual void enable() = 0;
    virtual void disable() = 0;

    operator bool() const { return tracing(); }

    static void globalEnable();
    static void globalDisable();
};

class SimpleFlag : public Flag
{
  protected:
    /** Whether this flag changes debug formatting. */
    const bool _isFormat = false;

    bool _enabled = false; // flag enablement status

    void sync() override { _tracing = _globalEnable && _enabled; }

  public:
    SimpleFlag(const char *name, const char *desc, bool is_format=false);

    void enable() override  { _enabled = true;  sync(); }
    void disable() override { _enabled = false; sync(); }

    /**
     * Checks whether this flag is a conventional debug flag, or a flag that
     * modifies the way debug information is printed.
     *
     * @return True if this flag is a debug-formatting flag.
     */
    bool isFormat() const { return _isFormat; }
};


namespace unions
{
    inline union InvalidReg
    {
        ~InvalidReg() {}
        SimpleFlag InvalidReg = {
            "InvalidReg", "", false
        };
    } InvalidReg;

    inline union CapRegs
    {
        ~CapRegs() {}
        SimpleFlag CapRegs = {
            "CapRegs", "", false
        };
    } CapRegs;
} // namespace unions


inline constexpr const auto& InvalidReg =
    ::debug::unions::InvalidReg.InvalidReg;

inline constexpr const auto& CapRegs =
    ::debug::unions::CapRegs.CapRegs;
}
#endif // __BASE_DEBUG_HH__

