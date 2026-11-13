#include "engine/camera.hpp"

#include <algorithm>
#include <cmath>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine
{
    namespace
    {
        constexpr float SPEED       = 2.5f;
        constexpr float SENSITIVITY = 0.1f;
        constexpr float ZOOM        = 45.0f;
    } // namespace

    Camera::Camera( glm::vec3 position, glm::vec3 up, float yaw, float pitch, float nearPlane, float farPlane )
        : m_position( position )
        , m_front( 0.0f, 0.0f, -1.0f )
        , m_worldUp( up )
        , m_yaw( yaw )
        , m_pitch( pitch )
        , m_movementSpeed( SPEED )
        , m_mouseSensitivity( SENSITIVITY )
        , m_zoom( ZOOM )
        , m_nearPlane( nearPlane )
        , m_farPlane( farPlane )
    {
        updateCameraVectors();

        auto& app     = Application::getInstance();
        m_mouseMotion = app.addMouseMotionCallback(
            [this]( const SDL_MouseMotionEvent& event )
            {
                if( event.state & SDL_BUTTON_RMASK )
                    processMouseMovement( event.xrel, -event.yrel );
            } );
        m_mouseWheel = app.addMouseWheelCallback( [this]( const SDL_MouseWheelEvent& event ) { processMouseScroll( event.y ); } );
        m_keyDown    = app.addKeyDownCallback( [this]( const SDL_KeyboardEvent& event ) { m_keys[event.scancode] = true; } );
        m_keyUp      = app.addKeyUpCallback( [this]( const SDL_KeyboardEvent& event ) { m_keys[event.scancode] = false; } );
        m_focusLost  = app.addWindowFocusLostCallback( [this]( const SDL_WindowEvent& ) { m_keys.fill( false ); } );
        m_update     = app.addUpdateCallback( [this]( float dt ) { updateInput( dt ); } );
    }

    glm::vec3 Camera::getPosition() const
    {
        return m_position;
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAtRH( m_position, m_position + m_front, m_up );
    }

    glm::mat4 Camera::getProjectionMatrix( float aspectRatio ) const
    {
        // Keep the original right-handed camera, with Metal's [0, 1] depth range.
        return glm::perspectiveRH_ZO( glm::radians( m_zoom ), aspectRatio, m_nearPlane, m_farPlane );
    }

    void Camera::setPosition( const glm::vec3& pos )
    {
        m_position = pos;
    }

    void Camera::processKeyboard( Movement direction, float deltaTime )
    {
        // Create forward vector that only tracks movement along xz-plane
        const glm::vec3 forward = glm::normalize( glm::vec3( m_front.x, 0.0f, m_front.z ) );
        const float velocity    = m_movementSpeed * deltaTime;

        switch( direction )
        {
            case Movement::FORWARD:
                m_position += forward * velocity;
                break;
            case Movement::BACKWARD:
                m_position -= forward * velocity;
                break;
            case Movement::LEFT:
                m_position -= m_right * velocity;
                break;
            case Movement::RIGHT:
                m_position += m_right * velocity;
                break;
            case Movement::UP:
                m_position += m_worldUp * velocity;
                break;
            case Movement::DOWN:
                m_position -= m_worldUp * velocity;
                break;
        }
    }

    void Camera::processMouseMovement( float xOffset, float yOffset, bool constrainPitch )
    {
        rotate( xOffset * m_mouseSensitivity, yOffset * m_mouseSensitivity, constrainPitch );
    }

    void Camera::rotate( float yawDegrees, float pitchDegrees, bool constrainPitch )
    {
        m_yaw += yawDegrees;
        m_pitch += pitchDegrees;

        if( constrainPitch )
        {
            m_pitch = std::clamp( m_pitch, -89.0f, 89.0f );
        }

        updateCameraVectors();
    }

    void Camera::processMouseScroll( float yOffset )
    {
        m_zoom = std::clamp( m_zoom - yOffset, 1.0f, 65.0f );
    }

    void Camera::updateInput( float dt )
    {
        const float movementDt = m_keys[SDL_SCANCODE_LSHIFT] ? dt * 2.0f : dt;

        if( m_keys[SDL_SCANCODE_W] )
            processKeyboard( Movement::FORWARD, movementDt );
        else if( m_keys[SDL_SCANCODE_S] )
            processKeyboard( Movement::BACKWARD, movementDt );

        if( m_keys[SDL_SCANCODE_A] )
            processKeyboard( Movement::LEFT, movementDt );
        else if( m_keys[SDL_SCANCODE_D] )
            processKeyboard( Movement::RIGHT, movementDt );

        if( m_keys[SDL_SCANCODE_E] )
            processKeyboard( Movement::UP, movementDt );
        else if( m_keys[SDL_SCANCODE_Q] )
            processKeyboard( Movement::DOWN, movementDt );

        const float turnStep = 60.0f * movementDt;
        float yaw            = 0.0f;
        float pitch          = 0.0f;
        if( m_keys[SDL_SCANCODE_LEFT] )
            yaw -= turnStep;
        if( m_keys[SDL_SCANCODE_RIGHT] )
            yaw += turnStep;
        if( m_keys[SDL_SCANCODE_UP] )
            pitch += turnStep;
        if( m_keys[SDL_SCANCODE_DOWN] )
            pitch -= turnStep;
        if( yaw != 0.0f || pitch != 0.0f )
            rotate( yaw, pitch );
    }

    void Camera::updateCameraVectors()
    {
        const float yawRadians   = glm::radians( m_yaw );
        const float pitchRadians = glm::radians( m_pitch );

        glm::vec3 front;
        front.x = std::cos( yawRadians ) * std::cos( pitchRadians );
        front.y = std::sin( pitchRadians );
        front.z = std::sin( yawRadians ) * std::cos( pitchRadians );

        m_front = glm::normalize( front );
        m_right = glm::normalize( glm::cross( m_front, m_worldUp ) );
        m_up    = glm::normalize( glm::cross( m_right, m_front ) );
    }
} // namespace engine
