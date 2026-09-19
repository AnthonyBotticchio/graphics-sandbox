#include "camera.hpp"
#include "shader.hpp"
#include "terrain.hpp"
#include "utils/render.hpp"
#include "utils/timers.hpp"
#include "utils/utils.hpp"

#include <memory>
#include <cmath>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
    float lastFrame = 0.0f; // Time of last frame

    static inline void mouse_callback( GLFWwindow* window, double x, double y )
    {
        auto* camera = static_cast<Camera*>( glfwGetWindowUserPointer( window ) );
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
        auto* camera = static_cast<Camera*>( glfwGetWindowUserPointer( window ) );
        if( camera != nullptr )
            camera->processMouseScroll( static_cast<float>( yOffset ) );
    }

    static inline void process_input( GLFWwindow* window, Camera& camera, float dt )
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
            camera.processKeyboard( Camera::Movement::FORWARD, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        {
            camera.processKeyboard( Camera::Movement::BACKWARD, movement_dt );
        }

        if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        {
            camera.processKeyboard( Camera::Movement::LEFT, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        {
            camera.processKeyboard( Camera::Movement::RIGHT, movement_dt );
        }

        if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
        {
            camera.processKeyboard( Camera::Movement::UP, movement_dt );
        }
        else if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
        {
            camera.processKeyboard( Camera::Movement::DOWN, movement_dt );
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

int main()
{
    // --- Init ---
    setup_logger( NULL, LOG_DEBUG, false );

    if( !glfwInit() )
    {
        log_error( "GLFW Init Failed" );
        return EXIT_FAILURE;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#ifdef __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE ); // Required on macOS
#endif

    GLFWwindow* window = glfwCreateWindow( WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL );
    if( !window )
    {
        log_error( "Failed to create GLFW window" );
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, utils::framebuffer_size_callback );

    std::unique_ptr<Camera> camera =
        std::make_unique<Camera>( glm::vec3( 0.0f, 0.0f, 3.0f ), glm::vec3( 0.0f, 2.0f, 0.0f ), -90.0f, 0.0f, 0.1f, 500.0f );
    glfwSetWindowUserPointer( window, camera.get() );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );

    if( glewInit() != GLEW_OK )
    {
        log_error( "GLEW Init Failed" );
        glfwDestroyWindow( window );
        glfwTerminate();
        return EXIT_FAILURE;
    }

    utils::display_device_info(); // Display startup info

    // --- Shaders ---

    Shader cubeShader( "vertex.vert", "fragment.frag" );
    Shader particleRenderShader( "particleRender.vert", "particle.frag" );
    Shader particleUpdateShader( "particleUpdate.vert", nullptr, { "oPosition", "oVelocity" } );
    Shader groundShader( "ground.vert", "ground.frag" );

    // --- Terrain ---

    constexpr unsigned int terrainCells = 500;
    std::unique_ptr<Terrain> terrain    = std::make_unique<Terrain>( terrainCells, 250.0f );

    // --- Setup ---

    // clang-format off
    // 6 faces of a cube
    constexpr GLfloat vertices[] = {
        // Positions         // Texture
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    constexpr GLuint indices[] = {
        0, 1, 4,  // First triangle
        1, 3, 4,  // Second triangle

        6, 2, 1,
        7, 0, 3
    };

    // Random cube positions in world coordinates
    constexpr std::array<glm::vec3, 10> cubePositions = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),   
        glm::vec3( 1.5f,  2.0f, -2.5f),  
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    constexpr std::array<float, 30> groundVertices = {
         // Position          // UV
        -1000, -1, -1000,        0, 0,
        -1000, -1,  1000,        0, 1,
        1000, -1,  1000,        1, 1,

        -1000, -1, -1000,        0, 0,
        1000, -1,  1000,        1, 1,
        1000, -1, -1000,        1, 0
    };
    // clang-format on

    // --- Cube Bindings ---

    GLuint vao, vbo, ebo;
    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &ebo );

    // VBO
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

    // VAO - must be set after VBO to be bound to it
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), nullptr ); // position attribute
    glEnableVertexAttribArray( 0 );
    // glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void*)( 3 * sizeof( float ) ) ); // color attribute
    // glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), reinterpret_cast<void*>( 3 * sizeof( float ) ) );
    glEnableVertexAttribArray( 1 );

    // EBO - must be set after VAO to be bound to it
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), &indices, GL_STATIC_DRAW );

    // We can unbind the VBO
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    // Textures
    GLuint wall_tex, saul_tex;
    utils::gen_texture( wall_tex, "wall.jpg" );
    utils::gen_texture( saul_tex, "saul.jpg" );

    // Constant Uniforms
    cubeShader.use();
    cubeShader.setUniform( "texture1", 0 );
    cubeShader.setUniform( "texture2", 1 );

    // --- Particle Bindings ---

    struct ParticleState
    {
        float position[3];
        float velocity[3];
    };

    constexpr GLsizei particleCount = 100000;
    std::vector<ParticleState> initialParticles( particleCount );

    std::mt19937 rng( 42 );
    std::uniform_real_distribution<float> positionDistribution( -1.0f, 1.0f );

    for( auto& particle : initialParticles )
    {
        for( int axis = 0; axis < 3; ++axis )
        {
            particle.position[axis] = positionDistribution( rng );
        }

        // Constant sideways drift makes the first test easy to recognize.
        particle.velocity[0] = 0.1f;
        particle.velocity[1] = 0.1f;
        particle.velocity[2] = 0.1f;
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

    // --- Ground Bindings ---

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
    utils::gen_texture( grass_tex, "saul.jpg" );

    // Render loop
    while( !glfwWindowShouldClose( window ) )
    {
        // UTILS_SCOPED_TIMER( "Render Block" )
        glfwPollEvents();

        float t   = static_cast<float>( glfwGetTime() );
        float dt  = t - lastFrame;
        lastFrame = t;

        process_input( window, *camera, dt );

        // Correct camera position
        glm::vec3 cameraPosition  = camera->getPosition();
        constexpr float eyeHeight = 1.3f;
        float minimumY            = terrain->getElevationAt( cameraPosition.x, cameraPosition.z ) + eyeHeight;
        cameraPosition.y          = std::max( minimumY, cameraPosition.y );
        camera->setPosition( cameraPosition );

        int fbW, fbH, windowW, windowH;
        glfwGetFramebufferSize( window, &fbW, &fbH );
        glfwGetWindowSize( window, &windowW, &windowH );

        // Skip rendering when minimized or without a drawable area.
        if( fbW <= 0 || fbH <= 0 || windowW <= 0 || windowH <= 0 )
            continue;

        utils::beginFrame( fbW, fbH );

        // Generic constants
        double mouseX, mouseY;
        glfwGetCursorPos( window, &mouseX, &mouseY );
        const float mx     = static_cast<float>( mouseX * fbW / windowW );
        const float my     = static_cast<float>( ( windowH - mouseY ) * fbH / windowH );
        const float aspect = static_cast<float>( fbW ) / fbH;

        // --- Ground ---

        utils::beginOpaquePass();

        groundShader.use();
        groundShader.setUniform( "groundTexture", 0 ); // Texture unit 0
        groundShader.setUniform( "cells", static_cast<float>( terrainCells ) );
        groundShader.setUniform( "model", glm::mat4( 1.0f ) );
        groundShader.setUniform( "view", camera->getViewMatrix() );
        groundShader.setUniform( "proj", camera->getProjectionMatrix( aspect ) );
        groundShader.setUniform( "t", t );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, grass_tex );

        glBindVertexArray( terrainVAO );
        glDrawElements( GL_TRIANGLES, terrain->getIndicesSize(), GL_UNSIGNED_INT, nullptr );

        // --- Cubes ---

        // utils::beginOpaquePass();

        // cubeShader.use();
        // cubeShader.setUniform( "view", camera->getViewMatrix() );
        // cubeShader.setUniform( "proj", camera->getProjectionMatrix( aspect ) );
        // cubeShader.setUniform( "theta", theta );
        // cubeShader.setUniform( "t", t );
        // cubeShader.setUniform( "mix_param", mix_param );

        // glActiveTexture( GL_TEXTURE0 );
        // glBindTexture( GL_TEXTURE_2D, wall_tex );
        // glActiveTexture( GL_TEXTURE1 );
        // glBindTexture( GL_TEXTURE_2D, saul_tex );

        // // Draw boxes
        // glBindVertexArray( vao );
        // for( size_t i{ cubePositions.size() }; i-- > 0; )
        // {
        //     glm::mat4 model = glm::translate( I4, cubePositions[i] );
        //     float angle     = 20.0f * i; // Provide a random angle
        //     model           = glm::rotate( model, glm::radians( angle ), glm::vec3( 1.0f, 0.3f, 0.5f ) );
        //     cubeShader.setUniform( "model", model );
        //     glDrawArrays( GL_TRIANGLES, 0, 36 );
        // }

        // --- Update Particle Pass ---

        // Unproject the cursor into a world-space ray, then intersect Z = 0.
        // A screen position alone has no depth, so this plane defines the target.
        const glm::mat4& view = camera->getViewMatrix();
        const glm::mat4& projection = camera->getProjectionMatrix( aspect );
        const glm::vec4 viewport( 0.0f, 0.0f, float( fbW ), float( fbH ) );
        const glm::vec3 rayStart = glm::unProject( glm::vec3( mx, my, 0.0f ), view, projection, viewport );
        const glm::vec3 rayEnd = glm::unProject( glm::vec3( mx, my, 1.0f ), view, projection, viewport );
        const glm::vec3 rayDirection = glm::normalize( rayEnd - rayStart );

        glm::vec3 mouseTarget( 0.0f );
        bool mouseActive = false;
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

        particleUpdateShader.use();
        particleUpdateShader.setUniform( "dt", std::min( dt, 0.033f ) );
        particleUpdateShader.setUniform( "acceleration", glm::vec3( 0.0f ) );
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

        utils::beginParticlePass();

        particleRenderShader.use();
        particleRenderShader.setUniform( "view", view );
        particleRenderShader.setUniform( "proj", projection );

        glBindVertexArray( renderVAOs[writeIndex] );
        glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, particleCount );

        // --- End Particle Pass ---

        std::swap( readIndex, writeIndex );

#ifdef __APPLE__
        glFinish(); // optional to synchronize draw calls. Reduces stuttering on OSX
#endif
        glfwSwapBuffers( window );
    }

    // de-allocate all resources once they've outlived their purpose
    const GLuint textures[] = { wall_tex, saul_tex, grass_tex };
    glDeleteTextures( static_cast<GLuint>( sizeof( textures ) / sizeof( GLuint ) ), textures );
    glDeleteVertexArrays( 1, &vao );
    glDeleteVertexArrays( 2, updateVAOs );
    glDeleteVertexArrays( 2, renderVAOs );
    glDeleteVertexArrays( 1, &terrainVAO );
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ebo );
    glDeleteBuffers( 2, particleBuffers );
    glDeleteBuffers( 1, &terrainVBO );
    glDeleteBuffers( 1, &terrainEBO );
    glDeleteProgram( cubeShader.getProgram() );
    glDeleteProgram( particleRenderShader.getProgram() );
    glDeleteProgram( particleUpdateShader.getProgram() );
    glDeleteProgram( groundShader.getProgram() );
    glfwDestroyWindow( window );

    glfwTerminate();

    log_info( "Window terminated successfully" );

    return EXIT_SUCCESS;
}
