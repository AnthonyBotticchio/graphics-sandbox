#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <functional>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace utilities
{
    void startup_info();
    void framebuffer_size_callback( GLFWwindow* window, int width, int height );
    std::string load_shader_source( const std::string& fileName );
    GLuint gen_shaders( const GLenum type, const GLsizei count, const char* const* shaderSource );
    GLuint gen_shader_program( const std::vector<GLuint>& shaders );
    void gen_texture( GLuint& texture, const std::string& path );
    bool shader_compile_check( const GLuint shader );
    bool shader_link_check( const GLuint shaderProgram );
    void time_block( std::function<void()> pred, const char* name = "Unknown" );
} // namespace utilities

#endif // UTILITIES_HPP
