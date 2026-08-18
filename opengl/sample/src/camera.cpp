#include "camera.hpp"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

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
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt( m_position, m_position + m_front, m_up );
}

glm::mat4 Camera::getProjectionMatrix( float aspectRatio ) const
{
    return glm::perspective( glm::radians( m_zoom ), aspectRatio, m_nearPlane, m_farPlane );
}

void Camera::processKeyboard( Movement direction, float deltaTime )
{
    const float velocity = m_movementSpeed * deltaTime;

    switch( direction )
    {
        case Movement::FORWARD:
            m_position += m_front * velocity;
            break;
        case Movement::BACKWARD:
            m_position -= m_front * velocity;
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
    m_yaw += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;

    if( constrainPitch )
        m_pitch = std::clamp( m_pitch, -89.0f, 89.0f );

    updateCameraVectors();
}

void Camera::processMouseScroll( float yOffset )
{
    m_zoom = std::clamp( m_zoom - yOffset, 1.0f, 65.0f );
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
