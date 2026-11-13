#ifndef ENGINE_CAMERA_HPP
#define ENGINE_CAMERA_HPP

#include "engine/application.hpp"

#include <array>

#include <glm/glm.hpp>

namespace engine
{
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

        explicit Camera( glm::vec3 position = glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3 up = glm::vec3( 0.0f, 1.0f, 0.0f ),
                         float yaw = -90.0f, float pitch = 0.0f, float nearPlane = 0.1f, float farPlane = 100.0f );
        ~Camera() = default;

        // Registered callbacks capture this camera's address.
        Camera( const Camera& )            = delete;
        Camera& operator=( const Camera& ) = delete;
        Camera( Camera&& )                 = delete;
        Camera& operator=( Camera&& )      = delete;

        [[nodiscard]] glm::vec3 getPosition() const;
        [[nodiscard]] glm::mat4 getViewMatrix() const;
        [[nodiscard]] glm::mat4 getProjectionMatrix( float aspectRatio ) const;

        void setPosition( const glm::vec3& pos );

        void processKeyboard( Movement direction, float deltaTime );
        void rotate( float yawDegrees, float pitchDegrees, bool constrainPitch = true );
        void processMouseMovement( float xOffset, float yOffset, bool constrainPitch = true );
        void processMouseScroll( float yOffset );

      private:
        void updateCameraVectors();
        void updateInput( float dt );

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

        std::array<bool, SDL_SCANCODE_COUNT> m_keys{};

        // Destroy connections before the camera state used by their callbacks.
        Application::MouseMotionEvent::Connection m_mouseMotion;
        Application::MouseWheelEvent::Connection m_mouseWheel;
        Application::KeyEvent::Connection m_keyDown;
        Application::KeyEvent::Connection m_keyUp;
        Application::WindowEvent::Connection m_focusLost;
        Application::UpdateEvent::Connection m_update;
    };
} // namespace engine

#endif // ENGINE_CAMERA_HPP
