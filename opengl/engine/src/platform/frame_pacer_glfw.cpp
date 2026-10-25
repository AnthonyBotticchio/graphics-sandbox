#include "frame_pacer_common.hpp"
#include "engine/frame_pacer.hpp"

namespace engine
{
    struct FramePacer::Impl
    {
        GLFWwindow* window;
    };

    FramePacer::FramePacer( GLFWwindow* window ) : m_impl( std::make_unique<Impl>( window ) )
    {
        glfwSwapInterval( 1 );
        log_info( "Frame pacing: GLFW VSync" );
    }

    FramePacer::~FramePacer() = default;

    bool FramePacer::waitForNextFrame()
    {
        glfwPollEvents();
        while( !glfwWindowShouldClose( m_impl->window ) )
        {
            if( hasDrawableArea( m_impl->window ) )
            {
                return true;
            }

            glfwWaitEventsTimeout( 0.1 );
        }
        return false;
    }
} // namespace engine
