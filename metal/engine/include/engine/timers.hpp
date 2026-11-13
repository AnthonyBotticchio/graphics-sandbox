#ifndef ENGINE_TIMERS_HPP
#define ENGINE_TIMERS_HPP

#include <chrono>

extern "C"
{
#include <log.h>
}

#define ENGINE_ENABLE_TIMER_LOGGING // Enables/disables logging entirely from timers
#define ENGINE_ENABLE_SCOPED_TIMERS // Enables creation of and output from scoped timers

#ifdef ENGINE_ENABLE_SCOPED_TIMERS
    #define ENGINE_DETAIL_CONCAT_INNER( a, b ) a##b
    #define ENGINE_DETAIL_CONCAT( a, b )       ENGINE_DETAIL_CONCAT_INNER( a, b )
    #define ENGINE_SCOPED_TIMER( name )                                                                                                    \
        [[maybe_unused]] const engine::ScopedTimer ENGINE_DETAIL_CONCAT( ScopedTimer_,                                                     \
                                                                         __LINE__ )( name ); // Creates a scoped timer with name 'name'
#else
    #define ENGINE_SCOPED_TIMER( name )
#endif

namespace engine
{
    /// @brief This is a RAII timer. Creates a timer which logs the amount of time since its creation and deletion. Typically defined at the
    /// top of scopes to log how long the body of the scope took.
    /// @note [[nodiscard]] fixes temporary ScopedTimer objects
    class [[nodiscard]] ScopedTimer
    {
      public:
        explicit ScopedTimer( const char* name = "Unknown" );
        virtual ~ScopedTimer();

        ScopedTimer( const ScopedTimer& )            = delete;
        ScopedTimer& operator=( const ScopedTimer& ) = delete;

      private:
        const char* m_name;
        std::chrono::steady_clock::time_point m_start;
    };
} // namespace engine


#endif // ENGINE_TIMERS_HPP
