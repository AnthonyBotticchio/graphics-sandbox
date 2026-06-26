#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <new>

extern "C"
{
#include <log.h>
}

/* Capacity = 4 (size = 1 << 4 = 0x0001 0000)
 * Capacity - 1 = 0x0000 1111
 * Index = 2 (incl. 0) -> 0x0000 0100
 * if((1 << Capcity) - 1 & (1 << new_idx) != 0x0); then idx.store(new_idx, std::memory_order::release);
 * new_idx = m_tail & 1 << (Capacity - 1) (0x0001 0000)
 *                        ^HEAD
 * if((1 << Capacity) - 1 & (1 << new_idx) != 0x0) -> false; then return false;
 */

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

    std::array<T, Capacity> m_buffer;
    alignas( std::hardware_destructive_interference_size ) std::atomic<size_t> m_head = 0;
    alignas( std::hardware_destructive_interference_size ) std::atomic<size_t> m_tail = 0;
};
#endif // SPSC_QUEUE_HPP