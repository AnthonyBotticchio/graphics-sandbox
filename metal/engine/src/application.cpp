#include "engine/application.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

extern "C"
{
#include <log.h>
}

namespace engine
{
    bool Application::initialize( int width, int height, const char* title )
    {
        if( m_initialized )
        {
            SDL_Log( "Application is already initialized" );
            return false;
        }

#ifdef __APPLE__
        SDL_SetHint( SDL_HINT_MAC_SCROLL_MOMENTUM, "1" );
#endif

        if( !SDL_Init( SDL_INIT_VIDEO ) )
        {
            reportSDLError( "SDL_Init" );
            SDL_Quit();
            return false;
        }
        m_initialized = true;

#ifdef __APPLE__
        m_window = SDL_CreateWindow( title, width, height, SDL_WINDOW_METAL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY );
        if( !m_window )
            return initializationFailed( "SDL_CreateWindow" );

        m_metalView = SDL_Metal_CreateView( m_window );
        if( !m_metalView )
            return initializationFailed( "SDL_Metal_CreateView" );

        m_layer = static_cast<CA::MetalLayer*>( SDL_Metal_GetLayer( m_metalView ) );
        if( !m_layer )
            return initializationFailed( "SDL_Metal_GetLayer" );

        log_info( "Metal application initialized" );
#endif

        m_lastFrameTicks = SDL_GetTicksNS();
        return true;
    }

    void Application::shutdown()
    {
        m_mouseMotion.clear();
        m_mouseButtonDown.clear();
        m_mouseButtonUp.clear();
        m_mouseWheel.clear();
        m_keyDown.clear();
        m_keyUp.clear();
        m_windowFocusLost.clear();
        m_update.clear();

        // GPU resources are released before their Metal view and window.
        if( auto* layer = std::exchange( m_layer, nullptr ) )
            layer->setDevice( nullptr );
        if( auto* view = std::exchange( m_metalView, nullptr ) )
            SDL_Metal_DestroyView( view );
        if( auto* window = std::exchange( m_window, nullptr ) )
            SDL_DestroyWindow( window );

        m_width          = 0;
        m_height         = 0;
        m_lastFrameTicks = 0;

        if( std::exchange( m_initialized, false ) ) // Reset initialized state
            SDL_Quit();
    }

    SDL_AppResult Application::handleEvent( const SDL_Event* event ) const
    {
        switch( event->type )
        {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                return SDL_APP_SUCCESS;
            case SDL_EVENT_KEY_DOWN:
                m_keyDown.dispatch( event->key );
                if( event->key.key == SDLK_ESCAPE )
                    return SDL_APP_SUCCESS;
                break;
            case SDL_EVENT_KEY_UP:
                m_keyUp.dispatch( event->key );
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                m_windowFocusLost.dispatch( event->window );
                break;
            case SDL_EVENT_MOUSE_MOTION:
                m_mouseMotion.dispatch( event->motion );
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                m_mouseButtonDown.dispatch( event->button );
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                m_mouseButtonUp.dispatch( event->button );
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                m_mouseWheel.dispatch( event->wheel );
                break;
            default:
                break;
        }

        return SDL_APP_CONTINUE;
    }

    Application::MouseMotionEvent::Connection Application::addMouseMotionCallback( MouseMotionEvent::Callback callback )
    {
        return m_mouseMotion.addCallback( std::move( callback ) );
    }

    Application::MouseButtonEvent::Connection Application::addMouseButtonDownCallback( MouseButtonEvent::Callback callback )
    {
        return m_mouseButtonDown.addCallback( std::move( callback ) );
    }

    Application::MouseButtonEvent::Connection Application::addMouseButtonUpCallback( MouseButtonEvent::Callback callback )
    {
        return m_mouseButtonUp.addCallback( std::move( callback ) );
    }

    Application::MouseWheelEvent::Connection Application::addMouseWheelCallback( MouseWheelEvent::Callback callback )
    {
        return m_mouseWheel.addCallback( std::move( callback ) );
    }

    Application::MouseButtonEvent::Connection Application::addMouseClickCallback( MouseButtonEvent::Callback callback, Uint8 button )
    {
        if( !callback )
            throw std::invalid_argument( "Mouse click callback must not be empty" );

        return addMouseButtonDownCallback(
            [callback = std::move( callback ), button]( const SDL_MouseButtonEvent& event )
            {
                if( event.button == button )
                    callback( event );
            } );
    }

    Application::MouseButtonEvent::Connection Application::addRightClickCallback( MouseButtonEvent::Callback callback )
    {
        return addMouseClickCallback( std::move( callback ), SDL_BUTTON_RIGHT );
    }

    SDL_AppResult Application::iterate()
    {
        if( !m_window )
            return SDL_APP_FAILURE;

        // Query pixels rather than window coordinates, including Retina/resize changes.
        if( !SDL_GetWindowSizeInPixels( m_window, &m_width, &m_height ) )
        {
            reportSDLError( "SDL_GetWindowSizeInPixels" );
            return SDL_APP_FAILURE;
        }

        const Uint64 now = SDL_GetTicksNS();
        const float dt   = std::min( static_cast<float>( now - m_lastFrameTicks ) / 1e9f, 0.033f ); // minimize steps to 0.033s
        m_lastFrameTicks = now;

        if( m_width <= 0 || m_height <= 0 || ( SDL_GetWindowFlags( m_window ) & SDL_WINDOW_MINIMIZED ) )
        {
            m_width = m_height = 0;
            SDL_Delay( 50 );
            return SDL_APP_CONTINUE;
        }

        m_update.dispatch( dt );
        return SDL_APP_CONTINUE;
    }

    Application::KeyEvent::Connection Application::addKeyDownCallback( KeyEvent::Callback callback )
    {
        return m_keyDown.addCallback( std::move( callback ) );
    }

    Application::KeyEvent::Connection Application::addKeyUpCallback( KeyEvent::Callback callback )
    {
        return m_keyUp.addCallback( std::move( callback ) );
    }

    Application::WindowEvent::Connection Application::addWindowFocusLostCallback( WindowEvent::Callback callback )
    {
        return m_windowFocusLost.addCallback( std::move( callback ) );
    }

    Application::UpdateEvent::Connection Application::addUpdateCallback( UpdateEvent::Callback callback )
    {
        return m_update.addCallback( std::move( callback ) );
    }

    int Application::drawableWidth() const
    {
        return m_width;
    }

    int Application::drawableHeight() const
    {
        return m_height;
    }

    CA::MetalLayer* Application::metalLayer() const
    {
        return m_layer;
    }

    Application::~Application()
    {
        shutdown();
    }

    void Application::reportSDLError( const char* operation )
    {
        SDL_Log( "%s: %s", operation, SDL_GetError() );
    }

    bool Application::initializationFailed( const char* operation )
    {
        reportSDLError( operation );
        shutdown();
        return false;
    }
} // namespace engine
