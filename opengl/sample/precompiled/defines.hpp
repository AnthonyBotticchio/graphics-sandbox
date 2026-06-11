#ifndef DEFINES_HPP
#define DEFINES_HPP

// c++ std includes
#include <concepts>
#include <utility>

// external
#include <glm/glm.hpp>

extern "C"
{
    #include <log.h>
}

// useful defines
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

inline constexpr glm::mat4 I4{ 1.0f };
inline constexpr glm::mat3 I3{ 1.0f };

#endif // DEFINES_HPP