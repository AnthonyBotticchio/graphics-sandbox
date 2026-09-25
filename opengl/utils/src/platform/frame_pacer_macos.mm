#include "frame_pacer_common.hpp"
#include "utils/frame_pacer.hpp"
#include "utils/timers.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <stdexcept>

#define GLFW_EXPOSE_NATIVE_COCOA
#import <CoreGraphics/CoreGraphics.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 140000
    #error FramePacer requires the macOS 14 SDK or newer.
#endif
#include <GLFW/glfw3native.h>
#import <QuartzCore/CADisplayLink.h>

@interface FramePacerDisplayLinkTarget : NSObject
{
  @public
    std::uint64_t ticks;
}
- (void)displayLinkDidFire:(CADisplayLink*)link;
@end

@implementation FramePacerDisplayLinkTarget

- (void)displayLinkDidFire:(CADisplayLink*)link
{
    ++ticks;
}

@end

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
        CADisplayLink* displayLink                     = nil;
        FramePacerDisplayLinkTarget* displayLinkTarget = nil;
        std::uint64_t handledTick                      = 0;
        bool waitingForRestoreTick                     = false;

        explicit Impl( GLFWwindow* value ) : window( value ) {}

        ~Impl() { [displayLink invalidate]; }

        void updateDisplayLinkRate()
        {
            NSWindow* nativeWindow = glfwGetCocoaWindow( window );
            NSScreen* screen       = nativeWindow.screen;
            if( !screen )
            {
                display     = kCGNullDirectDisplay;
                refreshRate = 0.0;
                interval    = Clock::duration::zero();
                nextFrame   = Clock::time_point{};
                return;
            }

            NSNumber* screenNumber                 = screen.deviceDescription[@"NSScreenNumber"];
            const CGDirectDisplayID currentDisplay = screenNumber ? screenNumber.unsignedIntValue : kCGNullDirectDisplay;
            const float rate                       = screen.maximumFramesPerSecond;
            if( !std::isfinite( rate ) || rate <= 0.0f || ( currentDisplay == display && rate == refreshRate ) )
                return;

            displayLink.preferredFrameRateRange = CAFrameRateRangeMake( rate, rate, rate );
            display                             = currentDisplay;
            refreshRate                         = rate;
            interval    = std::max( Clock::duration( 1 ),
                                    std::chrono::duration_cast<Clock::duration>( std::chrono::duration<double>( 1.0 / refreshRate ) ) );
            nextFrame   = Clock::time_point{};
            handledTick = displayLinkTarget->ticks;
            log_info( "Frame pacing: %.0f FPS target (display %u)", rate, display );
        }
    };

    FramePacer::FramePacer( GLFWwindow* window ) : m_impl( std::make_unique<Impl>( window ) )
    {
        if( !@available( macOS 14.0, * ) )
        {
            log_error( "FramePacer requires macOS 14 or newer" );
            throw std::runtime_error( "FramePacer requires macOS 14 or newer" );
        }

        glfwSwapInterval( 0 );

        NSWindow* nativeWindow    = glfwGetCocoaWindow( window );
        m_impl->displayLinkTarget = [FramePacerDisplayLinkTarget new];
        m_impl->displayLink = [nativeWindow displayLinkWithTarget:m_impl->displayLinkTarget selector:@selector( displayLinkDidFire: )];
        if( !m_impl->displayLink )
        {
            log_error( "Failed to create a display link for the window" );
            throw std::runtime_error( "Failed to create a display link for the window" );
        }

        m_impl->updateDisplayLinkRate();
        [m_impl->displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
        log_info( "Frame pacing: window display link" );
    }

    FramePacer::~FramePacer() = default;

    bool FramePacer::waitForNextFrame()
    {
        // UTILS_SCOPED_TIMER( "waitForNextFrame" )

        while( true )
        {
            glfwPollEvents();
            m_impl->updateDisplayLinkRate();
            if( glfwWindowShouldClose( m_impl->window ) )
                return false;

            if( !hasDrawableArea( m_impl->window ) )
            {
                m_impl->handledTick           = m_impl->displayLinkTarget->ticks;
                m_impl->waitingForRestoreTick = true;
                m_impl->nextFrame             = Impl::Clock::time_point{};
                glfwWaitEventsTimeout( 0.05 );
                continue;
            }

            if( m_impl->waitingForRestoreTick )
            {
                m_impl->handledTick           = m_impl->displayLinkTarget->ticks;
                m_impl->waitingForRestoreTick = false;
            }

            if( m_impl->interval == Impl::Clock::duration::zero() )
            {
                glfwWaitEventsTimeout( 0.05 );
                continue;
            }

            const auto now = Impl::Clock::now();
            if( m_impl->nextFrame == Impl::Clock::time_point{} )
            {
                if( m_impl->displayLinkTarget->ticks == m_impl->handledTick )
                {
                    glfwWaitEventsTimeout( 0.005 );
                    continue;
                }

                // Start a new frame schedule on a display-link callback.
                m_impl->handledTick = m_impl->displayLinkTarget->ticks;
                m_impl->nextFrame   = now;
            }

            if( now >= m_impl->nextFrame )
            {
                const auto elapsedSlots = ( now - m_impl->nextFrame ) / m_impl->interval;
                m_impl->nextFrame += m_impl->interval * ( elapsedSlots + 1 );
                return true;
            }

            const double wait = std::chrono::duration<double>( m_impl->nextFrame - now ).count();
            glfwWaitEventsTimeout( std::min( wait, 0.005 ) );
        }
    }
} // namespace utils
