#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <new>

extern "C"
{
#include <log.h>
}

enum class QueueType
{
    SPSC,
    SPMC,
    MPSC,
    MPMC
};

template<typename DataType, QueueType Queueing>
class Queue
{

  private:
    static constexpr size_t Align = std::hardware_destructive_interference_size; // Avoids false sharing and improves cache coherency

    alignas( Align ) std::atomic<size_t> m_head = 0;
    alignas( Align ) std::atomic<size_t> m_tail = 0;
    alignas( Align ) std::atomic<size_t> m_size = 0;       // If its dynamic
    alignas( Align ) std::byte* m_storage       = nullptr; // Object memory size, we'd want to constrain this if necessary

    int m_capacity = 0;
    int m_indexEnd;
};

/// @brief lock-free single-producer single-consumer queue
/// @param Capacity must be a power of two
template<class T, size_t Capacity>
    requires( Capacity != 0 && ( Capacity & ( Capacity - 1 ) ) == 0 )
class SPSCQueue
{
  public:
    SPSCQueue() = default;

    virtual ~SPSCQueue() = default;

    bool push( const T& item )
    {
        size_t currentTail = m_tail.load( std::memory_order::relaxed );
        size_t nextTail    = ( currentTail + 1 ) & BitMask;
        log_trace( "Current Tail: %zu", currentTail );
        log_trace( "Next Tail: %zu", nextTail );

        if( nextTail == m_head.load( std::memory_order::acquire ) )
            return false;

        m_buffer[currentTail] = item;
        m_tail.store( nextTail, std::memory_order::release );
        return true;
    }

    bool pop( T& value )
    {
        size_t currentHead = m_head.load( std::memory_order::relaxed );

        if( currentHead == m_tail.load( std::memory_order::acquire ) )
            return false; // buffer empty

        value = m_buffer[currentHead];
        m_head.store( ( currentHead + 1 ) & BitMask, std::memory_order::release );
        return true;
    }

    template<typename... ArgumentTypes>
    bool emplace( ArgumentTypes&&... arguments ); // new T(std::forward<ArgumentTypes>(arguments)...)

  private:
    static constexpr size_t BitMask = Capacity - 1;
    static constexpr size_t Align   = std::hardware_destructive_interference_size; // Avoids false sharing and improves cache coherency

    std::array<T, Capacity> m_buffer;
    alignas( Align ) std::atomic<size_t> m_head = 0;
    alignas( Align ) std::atomic<size_t> m_tail = 0;
    alignas( Align ) std::atomic<size_t> m_size = 0;       // If its dynamic
    alignas( Align ) std::byte* m_storage       = nullptr; // Object memory size, we'd want to constrain this if necessary
};

#endif // SPSC_QUEUE_HPP
