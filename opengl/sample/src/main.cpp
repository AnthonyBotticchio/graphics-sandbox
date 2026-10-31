#include "engine/camera.hpp"
#include "engine/frame_pacer.hpp"
#include "engine/renderer.hpp"
#include "engine/shader_program.hpp"
#include "engine/texture.hpp"
#include "engine/timers.hpp"
#include "engine/window.hpp"
#include "terrain.hpp"

#include <cmath>
#include <cstdlib>
#include <memory>
#include <random>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/ext.hpp>

namespace
{
    int HEIGHT      = 900;
    int WIDTH       = 1600;
    double lastX    = static_cast<double>( WIDTH ) / 2.0;
    double lastY    = static_cast<double>( HEIGHT ) / 2.0;
    bool firstMouse = true;
    float dX        = 0.00f;
    float dY        = 0.00f;
    float dZ        = 0.00f;
    float theta     = 0.00f;
    float mix_param = 0.00f;

    static inline void mouse_callback( GLFWwindow* window, double x, double y )
    {
        auto* camera = static_cast<engine::Camera*>( glfwGetWindowUserPointer( window ) );
        if( camera == nullptr )
            return;

        if( firstMouse )
        {
            lastX      = x;
            lastY      = y;
            firstMouse = false;
        }

        auto x_offset = static_cast<float>( x - lastX );
        auto y_offset = static_cast<float>( lastY - y ); // reversed since y-coordinates range from bottom to top
        lastX         = x;
        lastY         = y;

        camera->processMouseMovement( x_offset, y_offset );
    }

    static inline void scroll_callback( GLFWwindow* window, [[maybe_unused]] double xOffset, double yOffset )
    {
        auto* camera = static_cast<engine::Camera*>( glfwGetWindowUserPointer( window ) );
        if( camera != nullptr )
            camera->processMouseScroll( static_cast<float>( yOffset ) );
    }

    static inline void process_input( GLFWwindow* window, engine::Camera& camera, float dt )
    {
        float movement_dt = dt;

        if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS )
        {
            movement_dt *= 2.0f; // Twice as fast when pressing 'sprint'
        }

        if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        {
            glfwSetWindowShouldClose( window, true );
        }

        if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::FORWARD, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::BACKWARD, movement_dt );
        }

        if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::LEFT, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::RIGHT, movement_dt );
        }

        if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::UP, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
        {
            camera.processKeyboard( engine::Camera::Movement::DOWN, movement_dt );
        }

        // Pan (yaw) and tilt (pitch), in degrees per second.
        const float turnStep = 60.0f * movement_dt;
        float yaw            = 0.0f;
        float pitch          = 0.0f;
        if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
            yaw -= turnStep;
        if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
            yaw += turnStep;
        if( glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS )
            pitch += turnStep;
        if( glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS )
            pitch -= turnStep;
        if( yaw != 0.0f || pitch != 0.0f )
            camera.rotate( yaw, pitch );

        if( glfwGetKey( window, GLFW_KEY_RIGHT_BRACKET ) == GLFW_PRESS )
        {
            mix_param += 0.01f;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT_BRACKET ) == GLFW_PRESS )
        {
            mix_param -= 0.01f;
        }
    }
} // namespace

static int runSample()
{
    // Declare the window first so all OpenGL resources are released before its context (due to RAII)
    engine::Window context( { WIDTH, HEIGHT, "LearnOpenGL" } );
    GLFWwindow* window = context.nativeHandle();

    auto camera =
        std::make_unique<engine::Camera>( glm::vec3( 0.0f, 0.0f, 3.0f ), glm::vec3( 0.0f, 2.0f, 0.0f ), -90.0f, 0.0f, 0.1f, 500.0f );
    glfwSetWindowUserPointer( window, camera.get() );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );

    // --- Renderer ---

    engine::Renderer renderer;

    // --- Shaders ---

    engine::ShaderProgram cubeShader( "vertex.vert", "fragment.frag" );
    engine::ShaderProgram particleRenderShader( "particleRender.vert", "particle.frag" );
    engine::ShaderProgram particleUpdateShader( "particleUpdate.vert", nullptr, { "oPosition", "oVelocity" } );
    engine::ShaderProgram terrainShader( "terrain.vert", "terrain.frag" );

    // --- Terrain ---

    constexpr unsigned int meshCells         = 512;
    constexpr unsigned int terrainResolution = 512;
    std::unique_ptr<Terrain> terrain         = std::make_unique<Terrain>( meshCells, terrainResolution, 250.0f );

    // --- Particle Bindings ---

    struct ParticleState
    {
        float position[3];
        float velocity[3];
    };

    constexpr GLsizei particleCount = 1000000;
    std::vector<ParticleState> initialParticles( particleCount );

    std::mt19937 rng( 42 );
    std::normal_distribution<float> positionDistribution( 0.0f, 1.0f );

    for( auto& particle : initialParticles )
    {
        for( int axis = 0; axis < 3; ++axis )
        {
            particle.position[axis] = positionDistribution( rng );
        }

        // Constant sideways drift makes the first test easy to recognize.
        particle.velocity[0] = 0.0f;
        particle.velocity[1] = 0.0f;
        particle.velocity[2] = 0.0f;
    }

    GLuint particleBuffers[2];
    GLuint updateVAOs[2];
    GLuint renderVAOs[2];

    glGenBuffers( 2, particleBuffers );
    glGenVertexArrays( 2, updateVAOs );
    glGenVertexArrays( 2, renderVAOs );

    int readIndex  = 0;
    int writeIndex = 1;

    for( int i = 0; i < 2; ++i )
    {
        glBindBuffer( GL_ARRAY_BUFFER, particleBuffers[i] );
        glBufferData( GL_ARRAY_BUFFER, static_cast<GLsizeiptr>( initialParticles.size() * sizeof( ParticleState ) ),
                      initialParticles.data(), GL_DYNAMIC_COPY );

        // Update pass: one vertex = one particle.
        glBindVertexArray( updateVAOs[i] );

        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( ParticleState ),
                               reinterpret_cast<void*>( offsetof( ParticleState, position ) ) );
        glVertexAttribDivisor( 0, 0 );

        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( ParticleState ),
                               reinterpret_cast<void*>( offsetof( ParticleState, velocity ) ) );
        glVertexAttribDivisor( 1, 0 );

        // Render pass: one instance = one particle.
        glBindVertexArray( renderVAOs[i] );

        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( ParticleState ),
                               reinterpret_cast<void*>( offsetof( ParticleState, position ) ) );
        glVertexAttribDivisor( 0, 1 );
    }

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    glfwSetCursorPosCallback( window, nullptr ); // Disable camera movement

    // --- Terrain Bindings ---

    // Height Texutre
    GLuint heightTexture;
    engine::gen_terrain_texture( heightTexture, terrain->getResolution(), terrain->getHeights().data() );

    // Terrain verticies
    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays( 1, &terrainVAO );
    glGenBuffers( 1, &terrainVBO );
    glGenBuffers( 1, &terrainEBO );

    glBindVertexArray( terrainVAO );

    glBindBuffer( GL_ARRAY_BUFFER, terrainVBO );
    glBufferData( GL_ARRAY_BUFFER, terrain->getArrayBufferWidth(), terrain->getVertices().data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, terrainEBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, terrain->getElementBufferWidth(), terrain->getIndices().data(), GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, terrain->getSizeOfVertexType(), terrain->getOffsetOfVertexPosition() );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, terrain->getSizeOfVertexType(), terrain->getOffsetOfVertexTexture() );
    glEnableVertexAttribArray( 1 );

    GLuint grass_tex;
    engine::gen_texture( grass_tex, "saul.jpg" );

    {
        engine::FramePacer framePacer( window );
        float lastFrame = static_cast<float>( glfwGetTime() ); // Time of last frame

        std::random_device rd;
        std::mt19937 gen( rd() );
        constexpr float phaseMin = 0.0f;
        constexpr float phaseMax = TWO_PI;
        std::uniform_real_distribution<float> distrib( phaseMin, phaseMax );
        float phase = distrib( gen );

        // Render loop
        while( framePacer.waitForNextFrame() )
        {
            ENGINE_SCOPED_TIMER( "Render Time" )

            float t   = static_cast<float>( glfwGetTime() );
            float dt  = std::min( t - lastFrame, 0.033f );
            lastFrame = t;

            process_input( window, *camera, dt );

            // For future render class idea
            // Input and simulation
            // particles.update( dt, mouseTarget, mouseActive );
            //
            // Drawing
            // renderer.beginFrame(fbW, fbH, *camera);
            // renderer.drawTerrain( terrainMesh, phase);
            // renderer.drawParticles( particles );
            // context.swapBuffers();

            // Correct camera position
            glm::vec3 cameraPosition  = camera->getPosition();
            constexpr float eyeHeight = 1.3f;
            float minimumY            = terrain->getElevationAt( cameraPosition.x, cameraPosition.z ) + eyeHeight;
            cameraPosition.y          = std::max( minimumY, cameraPosition.y );
            camera->setPosition( cameraPosition );

            int fbW, fbH, windowW, windowH;
            glfwGetFramebufferSize( window, &fbW, &fbH );
            glfwGetWindowSize( window, &windowW, &windowH );

            renderer.beginFrame( fbW, fbH );

            // --- Constants ---

            double mouseX, mouseY;
            glfwGetCursorPos( window, &mouseX, &mouseY );
            const float mx             = static_cast<float>( mouseX * fbW / windowW );
            const float my             = static_cast<float>( ( windowH - mouseY ) * fbH / windowH );
            const float aspect         = static_cast<float>( fbW ) / fbH;
            const glm::mat4 view       = camera->getViewMatrix();
            const glm::mat4 projection = camera->getProjectionMatrix( aspect );
            const glm::mat4 projView   = projection * view;

            // --- Terrain ---

            constexpr float duration = 30.0f;
            phase += TWO_PI * ( dt / duration );
            phase = std::fmod( phase, TWO_PI );

            renderer.beginOpaquePass();

            terrainShader.use();
            terrainShader.setUniform( "terrainTexture", 0 ); // Texture unit 0
            terrainShader.setUniform( "model", glm::mat4( 1.0f ) );
            terrainShader.setUniform( "projView", projView );
            terrainShader.setUniform( "phase", phase );

            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, grass_tex );

            glBindVertexArray( terrainVAO );
            glDrawElements( GL_TRIANGLES, static_cast<GLsizei>( terrain->getIndicesSize() ), GL_UNSIGNED_INT, nullptr );

            // --- Update Particle Pass ---

            // Unproject the cursor into a world-space ray, then intersect Z = 0.
            // A screen position alone has no depth, so this plane defines the target.
            const glm::vec4 viewport( 0.0f, 0.0f, float( fbW ), float( fbH ) );
            const glm::vec3 rayStart     = glm::unProject( glm::vec3( mx, my, 0.0f ), view, projection, viewport );
            const glm::vec3 rayEnd       = glm::unProject( glm::vec3( mx, my, 1.0f ), view, projection, viewport );
            const glm::vec3 rayDirection = glm::normalize( rayEnd - rayStart );

            glm::vec3 mouseTarget( 0.0f );
            bool mouseActive        = false;
            const bool cursorInside = mouseX >= 0.0 && mouseX < windowW && mouseY >= 0.0 && mouseY < windowH;
            if( cursorInside && glfwGetWindowAttrib( window, GLFW_FOCUSED ) &&
                glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS && std::abs( rayDirection.z ) > 0.001f )
            {
                const float distance = -rayStart.z / rayDirection.z;
                // Ignore intersections behind the camera or beyond the visible ray.
                if( distance >= 0.0f && distance <= glm::length( rayEnd - rayStart ) )
                {
                    mouseTarget = rayStart + distance * rayDirection;
                    mouseActive = true;
                }
            }

            // Use std::min(dt, 0.033f) to prevent massive time steps during render times
            particleUpdateShader.use();

            glActiveTexture( GL_TEXTURE1 );
            glBindTexture( GL_TEXTURE_2D, heightTexture );

            particleUpdateShader.setUniform( "heightmap", 1 ); // Texture unit 1
            particleUpdateShader.setUniform( "terrainMin", terrain->getMinXZ() );
            particleUpdateShader.setUniform( "terrainSize", terrain->getSize() );
            particleUpdateShader.setUniform( "dt", dt );
            particleUpdateShader.setUniform( "acceleration", glm::vec3( 0.0f, -0.0f, 0.0f ) );
            particleUpdateShader.setUniform( "mouseTarget", mouseTarget );
            particleUpdateShader.setUniform( "mouseActive", mouseActive ? 1 : 0 );
            particleUpdateShader.setUniform( "attractionStrength", 2.0f );

            glBindVertexArray( updateVAOs[readIndex] );

            glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffers[writeIndex] );
            glEnable( GL_RASTERIZER_DISCARD );

            glBeginTransformFeedback( GL_POINTS );
            glDrawArrays( GL_POINTS, 0, particleCount );
            glEndTransformFeedback();

            glDisable( GL_RASTERIZER_DISCARD );
            glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0 );

            // --- Render Paritcle Pass ---

            renderer.beginParticlePass();

            particleRenderShader.use();
            particleRenderShader.setUniform( "projView", projView );
            particleRenderShader.setUniform( "view", view );

            glBindVertexArray( renderVAOs[writeIndex] );
            glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, particleCount );

            // --- End Particle Pass ---

            std::swap( readIndex, writeIndex );

            // --- End Draw ---

            context.swapBuffers();
        }

    } // Stop frame callbacks before releasing OpenGL resources and the window.

    // de-allocate all resources once they've outlived their purpose
    const GLuint textures[] = { grass_tex, heightTexture };
    glDeleteTextures( static_cast<GLuint>( sizeof( textures ) / sizeof( GLuint ) ), textures );
    glDeleteVertexArrays( 2, updateVAOs );
    glDeleteVertexArrays( 2, renderVAOs );
    glDeleteVertexArrays( 1, &terrainVAO );
    glDeleteBuffers( 2, particleBuffers );
    glDeleteBuffers( 1, &terrainVBO );
    glDeleteBuffers( 1, &terrainEBO );
    return EXIT_SUCCESS;
}

int main()
{
    setup_logger( nullptr, LOG_DEBUG, false );

    const int result = runSample();
    log_info( "Window terminated successfully" );
    return result;
}
