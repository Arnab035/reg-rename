#include <sstream>

#include "reg_class.hh"

namespace workflow
{

std::string
RegClassOps::regName(const RegId &id) const
{
    std::stringstream ss;
    ss << id.index();
    return ss.str();
}

std::string
RegClassOps::valString(const void *val, size_t size) const
{
    return "TODO: implemented later!";
}

}
