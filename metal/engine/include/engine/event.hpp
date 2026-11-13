#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace engine
{
    // A synchronous callback list. Register, disconnect, and dispatch on the same thread.
    template<class... Args>
    class Event
    {
      public:
        using Callback = std::function<void( Args... )>;

      private:
        struct Slot
        {
            Callback callback;
            bool connected = true;
        };

        struct State
        {
            std::vector<std::shared_ptr<Slot>> slots;
        };

      public:
        // Keep this connection alive for as long as the callback should be registered.
        class [[nodiscard]] Connection
        {
            friend class Event;

          public:
            Connection() = default;

            ~Connection() { disconnect(); }

            Connection( const Connection& )            = delete;
            Connection& operator=( const Connection& ) = delete;
            Connection( Connection&& ) noexcept        = default;

            Connection& operator=( Connection&& other ) noexcept
            {
                if( this != &other )
                {
                    disconnect();
                    m_state = std::move( other.m_state );
                    m_slot  = std::move( other.m_slot );
                }
                return *this;
            }

            void disconnect() noexcept
            {
                if( auto slot = m_slot.lock() )
                {
                    slot->connected = false;
                    if( auto state = m_state.lock() )
                        std::erase( state->slots, slot );
                }
                m_slot.reset();
                m_state.reset();
            }

            [[nodiscard]] bool connected() const noexcept
            {
                const auto slot = m_slot.lock();
                return !m_state.expired() && slot && slot->connected;
            }

          private:
            Connection( const std::shared_ptr<State>& state, const std::shared_ptr<Slot>& slot ) : m_state( state ), m_slot( slot ) {}

            std::weak_ptr<State> m_state;
            std::weak_ptr<Slot> m_slot;
        };

        Event() = default;

        ~Event() { clear(); }

        Event( const Event& )            = delete;
        Event& operator=( const Event& ) = delete;
        Event( Event&& )                 = delete;
        Event& operator=( Event&& )      = delete;

        [[nodiscard]] Connection addCallback( Callback callback )
        {
            if( !callback )
                throw std::invalid_argument( "Event callback must not be empty" );

            std::lock_guard<std::mutex> lock( m_mutex );
            auto slot = std::make_shared<Slot>( Slot{ std::move( callback ) } );
            m_state->slots.push_back( slot );
            return Connection( m_state, slot );
        }

        // Invoke in registration order. Additions wait until the next dispatch;
        // disconnected callbacks are skipped, even during an active dispatch.
        // Arguments are shared with all listeners; exceptions propagate to the caller.
        void dispatch( Args... args ) const
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            for( const auto& slot : m_state->slots )
            {
                if( slot->connected )
                    slot->callback( args... );
            }
        }

        void clear() noexcept
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            const auto& slots = std::exchange( m_state->slots, {} );
            for( const auto& slot : slots )
                slot->connected = false;
        }

      private:
        mutable std::mutex m_mutex;
        std::shared_ptr<State> m_state = std::make_shared<State>();
    };
} // namespace engine

#endif // ENGINE_EVENT_H
