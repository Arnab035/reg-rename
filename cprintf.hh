
#include <cassert>
#include <sstream>


std::pair<std::uint64_t, bool>
getUintX(const void *buf, std::size_t bytes, ByteOrder endian)
{
    assert(buf);
    switch (bytes) {
      case sizeof(std::uint64_t):
        return {gtoh(*(const std::uint64_t *)buf, endian), true};
      case sizeof(std::uint32_t):
        return {gtoh(*(const std::uint32_t *)buf, endian), true};
      case sizeof(std::uint16_t):
        return {gtoh(*(const std::uint16_t *)buf, endian), true};
      case sizeof(std::uint8_t):
        return {gtoh(*(const std::uint8_t *)buf, endian), true};
      default:
        return {0, false};
    }
}

std::pair<std::string, bool>
printUintX(const void *buf, std::size_t bytes, ByteOrder endian)
{
    auto [val, success] = getUintX(buf, bytes, endian);
    if (!success)
        return {"", false};

    std::ostringstream out;
    out << "0x";
    out.width(2 * bytes);
    out.fill('0');
    out.setf(std::ios::hex, std::ios::basefield);
    out << val;

    return {out.str(), true};
}

