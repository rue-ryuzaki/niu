#ifndef _NIU_ENDIAN_H_
#define _NIU_ENDIAN_H_

#include <cstdint>

#undef htonl
#undef htons
#undef ntohl
#undef ntohs

namespace niu {
// -- endian ------------------------------------------------------------------
inline bool
is_little_endian()
{
    uint16_t x = 0x0001;
    return *(reinterpret_cast<uint8_t*>(&x));
}

inline uint32_t
htonl(uint32_t hostlong)
{
    if (is_little_endian()) {
        return ((hostlong & 0xff) << 24)
             | ((hostlong & 0xff00) << 8)
             | ((hostlong & 0xff0000) >> 8)
             | ((hostlong & 0xff000000) >> 24);
    } else {
        return hostlong;
    }
}

inline uint16_t
htons(uint16_t hostshort)
{
    if (is_little_endian()) {
        return static_cast<uint16_t>(
                    (hostshort & 0xff) << 8) | ((hostshort & 0xff00) >> 8);
    } else {
        return hostshort;
    }
}

inline uint32_t
ntohl(uint32_t netlong)
{
    if (is_little_endian()) {
        return ((netlong & 0xff) << 24)
             | ((netlong & 0xff00) << 8)
             | ((netlong & 0xff0000) >> 8)
             | ((netlong & 0xff000000) >> 24);
    } else {
        return netlong;
    }
}

inline uint16_t
ntohs(uint16_t netshort)
{
    if (is_little_endian()) {
        return static_cast<uint16_t>(
                    (netshort & 0xff) << 8) | ((netshort & 0xff00) >> 8);
    } else {
        return netshort;
    }
}
}  // namespace niu

#endif  // _NIU_ENDIAN_H_
