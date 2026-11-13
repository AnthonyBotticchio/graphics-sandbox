#ifndef ENGINE_APPLICATION_HPP
#define ENGINE_APPLICATION_HPP

#include "engine/event.hpp"
#include "engine/singleton.hpp"

#include <QuartzCore/QuartzCore.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

namespace engine
{
    class Application final : public Singleton<Application>
    {
        friend class Singleton<Application>;

      public:
        using MouseMotionEvent = Event<const SDL_MouseMotionEvent&>;
        using MouseButtonEvent = Event<const SDL_MouseButtonEvent&>;
        using MouseWheelEvent  = Event<const SDL_MouseWheelEvent&>;
        using KeyEvent         = Event<const SDL_KeyboardEvent&>;
        using WindowEvent      = Event<const SDL_WindowEvent&>;
        using UpdateEvent      = Event<float>;

        [[nodiscard]] bool initialize( int width, int height, const char* title = "Metal C++ Sample" );
        void shutdown();
        [[nodiscard]] SDL_AppResult handleEvent( const SDL_Event* event ) const;
        [[nodiscard]] SDL_AppResult iterate();

        [[nodiscard]] MouseMotionEvent::Connection addMouseMotionCallback( MouseMotionEvent::Callback callback );
        [[nodiscard]] MouseButtonEvent::Connection addMouseButtonDownCallback( MouseButtonEvent::Callback callback );
        [[nodiscard]] MouseButtonEvent::Connection addMouseButtonUpCallback( MouseButtonEvent::Callback callback );
        [[nodiscard]] MouseWheelEvent::Connection addMouseWheelCallback( MouseWheelEvent::Callback callback );
        [[nodiscard]] MouseButtonEvent::Connection addMouseClickCallback( MouseButtonEvent::Callback callback, uint8_t button );
        [[nodiscard]] MouseButtonEvent::Connection addRightClickCallback( MouseButtonEvent::Callback callback );
        [[nodiscard]] KeyEvent::Connection addKeyDownCallback( KeyEvent::Callback callback );
        [[nodiscard]] KeyEvent::Connection addKeyUpCallback( KeyEvent::Callback callback );
        [[nodiscard]] WindowEvent::Connection addWindowFocusLostCallback( WindowEvent::Callback callback );
        [[nodiscard]] UpdateEvent::Connection addUpdateCallback( UpdateEvent::Callback callback );

        int drawableWidth() const;
        int drawableHeight() const;
        [[nodiscard]] CA::MetalLayer* metalLayer() const;

      private:
        Application() = default;
        ~Application() override;
        static void reportSDLError( const char* operation );
        bool initializationFailed( const char* operation );

        SDL_Window* m_window      = nullptr;
        SDL_MetalView m_metalView = nullptr;
        CA::MetalLayer* m_layer   = nullptr; // Borrowed from the SDL Metal view.
        int m_width               = 0;
        int m_height              = 0;
        bool m_initialized        = false;
        Uint64 m_lastFrameTicks   = 0;

        MouseMotionEvent m_mouseMotion;
        MouseButtonEvent m_mouseButtonDown;
        MouseButtonEvent m_mouseButtonUp;
        MouseWheelEvent m_mouseWheel;
        KeyEvent m_keyDown;
        KeyEvent m_keyUp;
        WindowEvent m_windowFocusLost;
        UpdateEvent m_update;
    };
} // namespace engine

#endif // ENGINE_APPLICATION_HPP
