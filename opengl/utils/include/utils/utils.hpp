#ifndef UTILS_HPP
#define UTILS_HPP

#include <filesystem>
#include <functional>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace utils
{
    std::filesystem::path get_runtime_dir();

    /// @brief Prints device and OGL information
    void display_device_info();

    void framebuffer_size_callback( GLFWwindow* window, int width, int height );

    void gen_texture( GLuint& texture, const std::string& path );

    void time_block( std::function<void()> pred, const char* name = "Unknown" );
} // namespace utils

#endif // UTILS_HPP
