#include "camera.hpp"

#include "utils/timers.hpp"
#include "utils/utils.hpp"

namespace
{
    constexpr float YAW         = -90.0f;
    constexpr float PITCH       = 0.0f;
    constexpr float SPEED       = 2.5f;
    constexpr float SENSITIVITY = 0.1f;
    constexpr float ZOOM        = 45.0f;
} // namespace

Camera::Camera()
{
    UTILS_SCOPED_TIMER( "Camera Constructor" )

    const auto i = std::to_underlying( m_movement );
}

Camera::Movement Camera::getMovement() const
{
    return m_movement;
}

const glm::vec4& Camera::getProjectionMatrix() const
{
    return glm::vec4{};
}
