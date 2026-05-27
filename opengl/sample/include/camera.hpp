#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera
{
    enum Camera_Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

  public:
    Camera();
    ~Camera() = default;

  private:
    glm::vec3 m_position;
};

#endif // CAMERA_HPP