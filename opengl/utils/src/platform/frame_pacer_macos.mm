#include "frame_pacer_common.hpp"
#include "utils/frame_pacer.hpp"
#include "utils/timers.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#define GLFW_EXPOSE_NATIVE_COCOA
#import <CoreGraphics/CoreGraphics.h>
#include <GLFW/glfw3native.h>

namespace utils
{
    struct FramePacer::Impl
    {
        using Clock = std::chrono::steady_clock;

        GLFWwindow* window;
        CGDirectDisplayID display = kCGNullDirectDisplay;
        double refreshRate        = 0.0;
        Clock::duration interval{};
        Clock::time_point nextFrame{};
        bool suspended = false;

        explicit Impl( GLFWwindow* value ) : window( value ) {}

        void updateDisplay()
        {
            @autoreleasepool
            {
                NSWindow* nativeWindow                 = glfwGetCocoaWindow( window );
                NSScreen* screen                       = nativeWindow.screen;
                NSNumber* screenNumber                 = screen.deviceDescription[@"NSScreenNumber"];
                const CGDirectDisplayID currentDisplay = screenNumber ? screenNumber.unsignedIntValue : kCGNullDirectDisplay;

                // Check screen rate if display is different
                if( currentDisplay != display )
                {
                    display     = currentDisplay;
                    double rate = 0.0;

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 120000
                    if( @available( macOS 12.0, * ) )
                        rate = screen.maximumFramesPerSecond;
                    else
#endif
                    {
                        CGDisplayModeRef mode =
                            currentDisplay != kCGNullDirectDisplay ? CGDisplayCopyDisplayMode( currentDisplay ) : nullptr;
                        if( mode )
                        {
                            rate = CGDisplayModeGetRefreshRate( mode );
                            CGDisplayModeRelease( mode );
                        }
                    }
                    if( !std::isfinite( rate ) || rate <= 0.0 )
                        rate = 60.0;

                    if( rate != refreshRate )
                    {
                        refreshRate = rate;
                        interval  = std::max( Clock::duration( 1 ),
                                              std::chrono::duration_cast<Clock::duration>( std::chrono::duration<double>( 1.0 / rate ) ) );
                        nextFrame = Clock::now();
                        log_info( "Frame pacing: steady-clock timer, %.2f FPS (display %u)", rate, display );
                    }
                }
            }
        }
    };

    FramePacer::FramePacer( GLFWwindow* window ) : m_impl( std::make_unique<Impl>( window ) )
    {
        glfwSwapInterval( 0 );
        m_impl->updateDisplay();
    }

    FramePacer::~FramePacer() = default;

    bool FramePacer::waitForNextFrame()
    {
        // UTILS_SCOPED_TIMER( "waitForNextFrame" )

        while( true )
        {
            using namespace std::chrono_literals;
            glfwPollEvents();
            if( glfwWindowShouldClose( m_impl->window ) )
                return false;
            m_impl->updateDisplay();

            if( !hasDrawableArea( m_impl->window ) )
            {
                m_impl->suspended = true;
                std::this_thread::sleep_until( Impl::Clock::now() + 50ms );
                continue;
            }

            const auto now = Impl::Clock::now();
            if( m_impl->suspended )
            {
                m_impl->nextFrame = now;
                m_impl->suspended = false;
            }
            if( now >= m_impl->nextFrame )
            {
                // Keep the original cadence, skipping missed slots instead of queuing frames.
                const auto elapsedSlots = ( now - m_impl->nextFrame ) / m_impl->interval;
                m_impl->nextFrame += m_impl->interval * ( elapsedSlots + 1 );
                return true;
            }

            // Bound input/close latency without spinning or adding render time to the interval.
            std::this_thread::sleep_until( std::min( m_impl->nextFrame, now + 10ms ) );
        }
    }
} // namespace utils
