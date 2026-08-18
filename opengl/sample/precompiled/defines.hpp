#ifndef DEFINES_HPP
#define DEFINES_HPP

// c++ std includes
#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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

constexpr glm::mat3 I3{ 1.0f };
constexpr glm::mat4 I4{ 1.0f };

#endif // DEFINES_HPP
