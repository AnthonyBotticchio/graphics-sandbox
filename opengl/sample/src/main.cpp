#include "camera.hpp"
#include "shader.hpp"
#include "utils/spsc_queue.hpp"
#include "utils/timers.hpp"
#include "utils/utils.hpp"

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>


namespace
{
    int HEIGHT      = 750;
    int WIDTH       = 1000;
    double lastX    = static_cast<double>( WIDTH ) / 2.0;
    double lastY    = static_cast<double>( HEIGHT ) / 2.0;
    bool firstMouse = true;
    float dX        = 0.00f;
    float dY        = 0.00f;
    float dZ        = 0.00f;
    float press_dur = 0.00f;
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
        float rotation_speed;
        float movement_dt = dt;

        if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS )
        {
            movement_dt *= 2.0f; // Twice as fast when pressing 'sprint'
            rotation_speed = 50.0f * dt;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_RELEASE )
        {
            rotation_speed = 25.0f * dt;
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

        if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
        {
            theta -= rotation_speed * ( 0.05f + press_dur );
            press_dur += 0.001f;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
        {
            theta += rotation_speed * ( 0.05f + press_dur );
            press_dur += 0.001f;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_RELEASE )
        {
            press_dur = 0.0f;
        }
        else if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_RELEASE )
        {
            press_dur = 0.0f;
        }

        if( glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS )
        {
            mix_param += 0.01f;
        }
        else if( glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS )
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

    std::unique_ptr<Camera> camera = std::make_unique<Camera>( glm::vec3( 0.0f, 0.0f, 3.0f ) );
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

    Shader myShader( "vertex.vert", "fragment.frag" );

    // --- Setup ---

    // clang-format off
    // constexpr GLfloat vertices[] = { 
    //     // positions         // colors           // texture
    //     0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    //     -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    //     0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f,   // top middle
    //     -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 1.0f,   0.0f, 1.0f,   // top left
    //     0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   // top right

    //     -0.5f,  0.5f, 0.0f,     // top left 
    //     0.0f,  0.5f, 0.0f,      // top middle
    //     0.0f, -0.5f, 0.0f,      // bottom middle

    //     0.0f, 1.0f, 0.0f,
    //     0.0f, -1.0f, 0.0f
    // };

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
    // clang-format on

    GLuint vao, vbo, ebo;
    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &ebo );

    // VBO
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

    // VAO - must be set after VBO to be bound to it
    glBindVertexArray( vao );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), (void*)0 ); // position attribute
    glEnableVertexAttribArray( 0 );
    // glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), (void*)( 3 * sizeof( float ) ) ); // color attribute
    // glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), (void*)( 3 * sizeof( float ) ) ); // texture attribute
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
    myShader.use();
    myShader.setUniform( "texture1", 0 );
    myShader.setUniform( "texture2", 1 );

    glEnable( GL_DEPTH_TEST );

    bool ok = true;
    SPSCQueue<int, 4> q;
    ok &= q.push( 10 );
    ok &= q.push( 21 );
    if( !ok )
        log_fatal( "push operations failed" );

    ok = true;
    int res1, res2;
    ok &= q.pop( res2 );
    ok &= q.pop( res1 );
    if( !ok )
        log_fatal( "pop operations failed" );

    log_info( "res1: %d, res2: %d", res1, res2 );

    // Render loop
    while( !glfwWindowShouldClose( window ) )
    {
        // UTILS_SCOPED_TIMER( "Render Block" )

        // Deterministic calculations before
        auto t    = static_cast<float>( glfwGetTime() );
        auto dt   = t - lastFrame;
        lastFrame = t;
        glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );
        auto ASPECT                 = static_cast<float>( WIDTH ) / static_cast<float>( HEIGHT );
        const glm::mat4& view       = camera->getViewMatrix();
        const glm::mat4& projection = camera->getProjectionMatrix( ASPECT );

        // Inputs: Check and call events and swap the buffers
        glfwSwapBuffers( window );
        glfwPollEvents();
        process_input( window, *camera, dt );

        // Colors and Depth
        glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Bind multiple textures
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, wall_tex );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, saul_tex );

        // Set dynamically changing uniforms
        myShader.setUniform( "theta", theta );
        myShader.setUniform( "t", t );
        myShader.setUniform( "mix_param", mix_param );
        myShader.setUniform( "view", view );
        myShader.setUniform( "proj", projection );

        // Draw boxes
        glBindVertexArray( vao );
        for( size_t i{ cubePositions.size() }; i-- > 0; )
        {
            glm::mat4 model = glm::translate( I4, cubePositions[i] );
            float angle     = 20.0f * i; // Provide a random angle
            model           = glm::rotate( model, glm::radians( angle ), glm::vec3( 1.0f, 0.3f, 0.5f ) );
            myShader.setUniform( "model", model );
            glDrawArrays( GL_TRIANGLES, 0, 36 );
        }

#ifdef __APPLE__
        glFinish(); // optional to synchronize draw calls. Reduces stuttering on OSX
#endif

        // Finish using our program. Only necessary if we are using multiple programs
        // glUseProgram(0);
    }

    // de-allocate all resources once they've outlived their purpose
    const GLuint textures[] = { wall_tex, saul_tex };
    glDeleteTextures( 2, textures );
    glDeleteVertexArrays( 1, &vao );
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ebo );
    glDeleteProgram( myShader.getProgram() );
    glfwDestroyWindow( window );

    glfwTerminate();

    log_info( "Window terminated successfully" );

    return EXIT_SUCCESS;
}
