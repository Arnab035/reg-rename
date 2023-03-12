#include "debug.hh"

namespace debug
{

Flag::Flag(const char *name, const char *desc)
    : _name(name), _desc(desc)
{}

bool Flag::_globalEnable = false;

SimpleFlag::SimpleFlag(const char *name, const char *desc, bool is_format)
  : Flag(name, desc), _isFormat(is_format)
{}

Flag::~Flag()
{}

}
