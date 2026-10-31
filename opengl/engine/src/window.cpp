#include "engine/window.hpp"

#include "engine/diagnostics.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

extern "C"
{
#include <log.h>
}

namespace
{
    bool windowActive = false;

    void framebufferSizeCallback( [[maybe_unused]] GLFWwindow* window, int width, int height )
    {
        glViewport( 0, 0, width, height );
        log_trace( "framebuffer callback - width: %d, height: %d", width, height );
    }

    void glfwErrorCallback( int code, const char* description )
    {
        log_error( "GLFW error %d: %s", code, description );
    }
} // namespace

namespace engine
{
    Window::Window( const WindowConfig& config )
    {
        if( windowActive )
        {
            throw std::runtime_error( "Only one engine Window may be active at a time" );
        }

        if( config.width <= 0 || config.height <= 0 )
        {
            throw std::runtime_error( "Window dimensions must be positive" );
        }

        glfwSetErrorCallback( glfwErrorCallback );

        if( !glfwInit() )
        {
            glfwTerminate();
            throw std::runtime_error( "GLFW initialization failed" );
        }

        glfwDefaultWindowHints();
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#ifdef __APPLE__
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
#endif

        m_window = glfwCreateWindow( config.width, config.height, config.title.c_str(), nullptr, nullptr );
        if( m_window == nullptr )
        {
            glfwTerminate();
            throw std::runtime_error( "Failed to create an OpenGL window" );
        }

        glfwMakeContextCurrent( m_window );
        if( !gladLoadGLLoader( reinterpret_cast<GLADloadproc>( glfwGetProcAddress ) ) || !GLAD_GL_VERSION_4_1 )
        {
            glfwDestroyWindow( m_window );
            m_window = nullptr;
            glfwTerminate();
            throw std::runtime_error( "Failed to load OpenGL 4.1 functions from GLAD" );
        }

        glfwSetFramebufferSizeCallback( m_window, framebufferSizeCallback );
        display_device_info();
        windowActive = true;
    }

    Window::~Window()
    {
        glfwDestroyWindow( m_window );
        glfwTerminate();
        windowActive = false;
    }

    GLFWwindow* Window::nativeHandle() const noexcept
    {
        return m_window;
    }

    void Window::swapBuffers() const
    {
        glfwSwapBuffers( m_window );
    }
} // namespace engine
