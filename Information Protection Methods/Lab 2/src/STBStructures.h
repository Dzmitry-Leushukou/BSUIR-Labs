#include <stdint.h>
#include <array>


using KEY = std::array<uint32_t, 8>;
using KTABLE = std::array<uint8_t, 256>;
using DBLOCK = std::array<uint32_t, 4>;
using SUBKEYS = std::array<uint32_t, 56>;
