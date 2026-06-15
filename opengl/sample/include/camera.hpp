#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera
{
    enum class Movement : u8
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

  public:
    Camera();
    ~Camera() = default;

    Movement getMovement() const;
    const glm::vec4& getProjectionMatrix() const;

    const glm::vec4& fromView( const glm::vec4& view );
    const glm::vec4& lookAt( const glm::mat3& camera );

  private:
    glm::vec4 m_projection;
    glm::vec4 m_view;
    glm::vec4 m_model;

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;

    Movement m_movement = Movement::FORWARD;
};

#endif // CAMERA_HPP