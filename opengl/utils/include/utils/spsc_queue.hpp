#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include "utils/timers.hpp"

#include <array>
#include <atomic>

/// @brief lock-free single-producer single-consumer queue
template<class T, size_t Capacity>
class SPSCQueue
{
  public:
    SPSCQueue() = default;

    virtual ~SPSCQueue() = default;

    bool push( const T& item )
    {
        size_t currentTail = m_tail.load( std::memory_order::relaxed );
        size_t nextTail    = ( currentTail + 1 ) % Capacity;

        if( nextTail == m_head.load( std::memory_order::acquire ) )
            return false;

        m_buffer[nextTail] = item;
        m_tail.store( nextTail, std::memory_order::release );
        return true;
    }

    bool pop( T& value )
    {
        size_t currentHead = m_head.load( std::memory_order::relaxed );

        if( currentHead == m_tail.load( std::memory_order::acquire ) )
            return false; // buffer empty

        value = m_buffer[currentHead];
        m_head.store( ( currentHead + 1 ) % Capacity, std::memory_order::release );
        return true;
    }

  private:
    std::array<T, Capacity> m_buffer;
    alignas( 64 ) std::atomic<size_t> m_head;
    alignas( 64 ) std::atomic<size_t> m_tail;
};

#endif // SPSC_QUEUE_HPP