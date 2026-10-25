#ifndef ENGINE_RUNTIME_HPP
#define ENGINE_RUNTIME_HPP

#include <filesystem>

namespace engine
{
    [[nodiscard]] std::filesystem::path get_runtime_dir();
} // namespace engine

#endif // ENGINE_RUNTIME_HPP
