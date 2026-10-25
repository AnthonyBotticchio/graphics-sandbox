#ifndef ENGINE_FRAME_PACER_HPP
#define ENGINE_FRAME_PACER_HPP

#include <memory>

struct GLFWwindow;

// TODO: Add a feature for a frame cap rather than using GLFW VSync/Apple DisplayLink for matching fps to a monitor's refresh rate. I
// suggest using an Enum, one for VSync/Matching display refresh rate, and another for suggested framerate.

namespace engine
{
    /// @brief Use on the main thread with the window's context current. Destroy before the window.
    class FramePacer
    {
      public:
        explicit FramePacer( GLFWwindow* window );
        virtual ~FramePacer();

        FramePacer( const FramePacer& )            = delete;
        FramePacer& operator=( const FramePacer& ) = delete;

        /// @brief Processes events and waits for a drawable frame. False means the window closed.
        bool waitForNextFrame();

      private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace engine

#endif // ENGINE_FRAME_PACER_HPP
