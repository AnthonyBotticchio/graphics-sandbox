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

  private:
    Movement m_movement = Movement::FORWARD;
    glm::vec4 m_position;
    glm::vec4 m_projection;
    glm::vec4 m_view;
    glm::vec4 m_model;
};

#endif // CAMERA_HPP