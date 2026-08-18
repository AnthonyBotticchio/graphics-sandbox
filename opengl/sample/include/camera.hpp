#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera
{
  public:
    enum class Movement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    explicit Camera( glm::vec3 position = glm::vec3( 0.0f, 0.0f, 0.0f ),
                     glm::vec3 up       = glm::vec3( 0.0f, 1.0f, 0.0f ),
                     float yaw          = -90.0f,
                     float pitch        = 0.0f,
                     float nearPlane    = 0.1f,
                     float farPlane     = 100.0f );
    ~Camera() = default;

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix( float aspectRatio ) const;

    void processKeyboard( Movement direction, float deltaTime );
    void processMouseMovement( float xOffset, float yOffset, bool constrainPitch = true );
    void processMouseScroll( float yOffset );

  private:
    void updateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;
    float m_nearPlane;
    float m_farPlane;
};

#endif // CAMERA_HPP
