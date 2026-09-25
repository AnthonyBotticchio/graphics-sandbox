#include "utils/timers.hpp"

namespace utils
{
    ScopedTimer::ScopedTimer( const char* name )
    {
        m_name  = name;
        m_start = std::chrono::steady_clock::now();
    }

    ScopedTimer::~ScopedTimer()
    {
        auto end      = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::milli>( end - m_start ); // Fractional milliseconds
#ifdef UTILS_ENABLE_TIMER_LOGGING
        log_trace( "%s : %.3f ms", m_name, duration.count() );
#endif
    }
} // namespace utils
