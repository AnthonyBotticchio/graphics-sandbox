#ifndef GPU_TIMER_HPP
#define GPU_TIMER_HPP

#include <array>
#include <cstddef>

#include <glad/glad.h>

// One timer per pass. GL_TIME_ELAPSED queries must never overlap.
class GpuTimer
{
  public:
    GpuTimer()
    {
        GLint bits = 0;
        glGetQueryiv( GL_TIME_ELAPSED, GL_QUERY_COUNTER_BITS, &bits );
        m_supported = bits > 0;
        if( m_supported )
            glGenQueries( static_cast<GLsizei>( m_queries.size() ), m_queries.data() );
    }

    ~GpuTimer()
    {
        if( m_supported )
            glDeleteQueries( static_cast<GLsizei>( m_queries.size() ), m_queries.data() );
    }

    GpuTimer( const GpuTimer& )            = delete;
    GpuTimer& operator=( const GpuTimer& ) = delete;

    // Poll once per frame, before issuing new measurements. Never wait for results.
    void collect()
    {
        for( std::size_t i = 0; i < m_queries.size(); ++i )
        {
            if( !m_pending[i] )
                continue;

            GLint available = GL_FALSE;
            glGetQueryObjectiv( m_queries[i], GL_QUERY_RESULT_AVAILABLE, &available );
            if( available == GL_FALSE )
                continue;

            GLuint64 nanoseconds = 0;
            glGetQueryObjectui64v( m_queries[i], GL_QUERY_RESULT, &nanoseconds );
            m_totalMilliseconds += static_cast<double>( nanoseconds ) / 1'000'000.0;
            ++m_samples;
            m_pending[i] = false;
        }
    }

    void begin()
    {
        if( !m_supported || m_pending[m_next] )
            return;
        glBeginQuery( GL_TIME_ELAPSED, m_queries[m_next] );
        m_active = true;
    }

    void end()
    {
        if( !m_active )
            return;
        glEndQuery( GL_TIME_ELAPSED );
        m_pending[m_next] = true;
        m_next            = ( m_next + 1 ) % m_queries.size();
        m_active          = false;
    }

    [[nodiscard]] bool supported() const { return m_supported; }

    [[nodiscard]] bool hasSamples() const { return m_samples > 0; }

    // Call only when hasSamples() is true.
    double takeAverageMilliseconds()
    {
        const double average = m_totalMilliseconds / static_cast<double>( m_samples );
        m_totalMilliseconds  = 0.0;
        m_samples            = 0;
        return average;
    }

  private:
    std::array<GLuint, 4> m_queries{};
    std::array<bool, 4> m_pending{};
    std::size_t m_next         = 0;
    std::size_t m_samples      = 0;
    double m_totalMilliseconds = 0.0;
    bool m_active              = false;
    bool m_supported           = false;
};

#endif // GPU_TIMER_HPP
