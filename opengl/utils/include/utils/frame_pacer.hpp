#ifndef UTILS_FRAME_PACER_HPP
#define UTILS_FRAME_PACER_HPP

#include <memory>

struct GLFWwindow;

namespace utils
{
    /// @brief Use on the main thread with the window's context current. Destroy before the window.
    class FramePacer
    {
      public:
        explicit FramePacer( GLFWwindow* window );
        ~FramePacer();

        FramePacer( const FramePacer& )            = delete;
        FramePacer& operator=( const FramePacer& ) = delete;

        /// @brief Processes events and waits for a drawable frame. False means the window closed.
        bool waitForNextFrame();

      private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace utils

#endif // UTILS_FRAME_PACER_HPP
