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

    const glm::vec4& getProjectionMatrix() const;

  private:
    glm::vec3 m_position;
};

#endif // CAMERA_HPP