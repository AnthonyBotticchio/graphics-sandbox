#ifndef ENGINE_WINDOW_HPP
#define ENGINE_WINDOW_HPP

#include <string>

struct GLFWwindow;

namespace engine
{
    struct WindowConfig
    {
        int width         = 1600;
        int height        = 900;
        std::string title = "OpenGL Sample";
    };

    /// @brief Owns GLFW and an OpenGL 4.1 context. Use one Window at a time, on the main thread.
    /// @note Destroy FramePacer and all OpenGL resources before destroying the Window.
    class Window
    {
      public:
        explicit Window( const WindowConfig& config = {} );
        ~Window();

        Window( const Window& )            = delete;
        Window& operator=( const Window& ) = delete;
        Window( Window&& )                 = delete;
        Window& operator=( Window&& )      = delete;

        [[nodiscard]] GLFWwindow* nativeHandle() const noexcept;
        void swapBuffers() const;

      private:
        GLFWwindow* m_window = nullptr;
    };
} // namespace engine

#endif // ENGINE_WINDOW_HPP
