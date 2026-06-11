#include "camera.hpp"
#include "utils/utils.hpp"

#include <array>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace
{
    int HEIGHT      = 750;
    int WIDTH       = 1000;
    double lastX    = WIDTH / 2;
    double lastY    = HEIGHT / 2;
    bool firstMouse = true;
    float yaw       = 0.0;
    float pitch     = 0.0;
    float FOV       = 60.0f;
    float dX        = 0.00f;
    float dY        = 0.00f;
    float dZ        = 0.00f;
    float press_dur = 0.00f;
    float theta     = 0.00f;
    float mix_param = 0.00f;
    float lastFrame = 0.0f; // Time of last frame

    glm::vec3 cameraPos   = glm::vec3( 0.0f, 0.0f, 3.0f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp    = glm::vec3( 0.0f, 1.0f, 0.0f );

    static inline void mouse_callback( GLFWwindow* window, double x, double y )
    {
        // if( firstMouse )
        // {
        //     lastX      = x;
        //     lastY      = y;
        //     firstMouse = false;
        // }

        float x_offset = x - lastX;
        float y_offset = lastY - y; // reversed since y-coordinates range from bottom to top
        lastX          = x;
        lastY          = y;

        constexpr float sensitivity = 0.1f;
        x_offset *= sensitivity;
        y_offset *= sensitivity;

        yaw += x_offset;
        pitch += y_offset;

        if( pitch > 89.0f )
        {
            pitch = 89.0f;
        }
        else if( pitch < -89.0f )
        {
            pitch = -89.0f;
        }

        // log_trace( "Captured mouse callback. X: %f. Y: %f", lastX, lastY );

        float c_yaw   = cos( glm::radians( yaw ) );
        float s_yaw   = sin( glm::radians( yaw ) );
        float c_pitch = cos( glm::radians( pitch ) );
        float s_pitch = sin( glm::radians( pitch ) );
        cameraFront   = glm::normalize( glm::vec3( c_yaw * c_pitch, s_pitch, s_yaw * c_pitch ) );
    }

    static inline void process_input( GLFWwindow* window, float dt )
    {
        float camera_speed;
        float rotation_speed;

        if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS )
        {
            camera_speed   = 5.0f * dt;
            rotation_speed = 50.0f * dt;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_RELEASE )
        {
            camera_speed   = 2.5f * dt;
            rotation_speed = 25.0f * dt;
        }

        if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        {
            glfwSetWindowShouldClose( window, true );
        }

        if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        {
            cameraPos += camera_speed * cameraFront;
        }
        else if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        {
            cameraPos -= camera_speed * cameraFront;
        }

        if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        {
            cameraPos -= camera_speed * glm::normalize( glm::cross( cameraFront, cameraUp ) );
        }
        else if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        {
            cameraPos += camera_speed * glm::normalize( glm::cross( cameraFront, cameraUp ) );
        }

        if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
        {
            cameraPos += camera_speed * cameraUp;
        }
        else if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
        {
            cameraPos -= camera_speed * cameraUp;
        }

        if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
        {
            theta -= rotation_speed * ( 0.05 + press_dur );
            press_dur += 0.001;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
        {
            theta += rotation_speed * ( 0.05 + press_dur );
            press_dur += 0.001;
        }
        else if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_RELEASE )
        {
            press_dur = 0.0;
        }
        else if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_RELEASE )
        {
            press_dur = 0.0;
        }

        if( glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS )
        {
            mix_param += 0.01;
        }
        else if( glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            mix_param -= 0.01;
        }

        if( glfwGetKey( window, GLFW_KEY_X ) == GLFW_PRESS )
        {
            FOV += 0.1;
        }
        else if( glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS )
        {
            FOV -= 0.1;
        }
    }


} // namespace

int main()
{
    // --- Init ---

    setup_logger( NULL, LOG_DEBUG, true );

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
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetCursorPosCallback( window, mouse_callback );

    if( glewInit() != GLEW_OK )
    {
        log_error( "GLEW Init Failed" );
        glfwDestroyWindow( window );
        glfwTerminate();
        return EXIT_FAILURE;
    }

    utils::display_device_info(); // Display startup info

    // --- Shaders ---

    const std::string vertex_source   = utils::load_shader_source( "vertex.vert" );
    const std::string fragment_source = utils::load_shader_source( "fragment.frag" );

    const char* vertex_source_ptr   = vertex_source.c_str();
    const char* fragment_source_ptr = fragment_source.c_str();

    log_debug( "Found Vertex Shader:\n%s", vertex_source_ptr );
    log_debug( "Found Fragment Shader:\n%s", fragment_source_ptr );

    const GLuint vertex_shader   = utils::gen_shaders( GL_VERTEX_SHADER, 1, &vertex_source_ptr );
    const GLuint fragment_shader = utils::gen_shaders( GL_FRAGMENT_SHADER, 1, &fragment_source_ptr );

    const std::vector<GLuint> shaders = { vertex_shader, fragment_shader }; // Must be in order
    const GLuint shader_prog          = utils::gen_shader_program( shaders );

    glUseProgram( shader_prog );

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
    GLuint thetaLoc     = glGetUniformLocation( shader_prog, "theta" );
    GLuint timeLoc      = glGetUniformLocation( shader_prog, "t" );
    GLuint mix_paramLoc = glGetUniformLocation( shader_prog, "mix_param" );
    GLuint modelLoc     = glGetUniformLocation( shader_prog, "model" );
    GLuint viewLoc      = glGetUniformLocation( shader_prog, "view" );
    GLuint projLoc      = glGetUniformLocation( shader_prog, "proj" );
    glUniform1i( glGetUniformLocation( shader_prog, "texture1" ), 0 ); // set texture1 to texture unit 0
    glUniform1i( glGetUniformLocation( shader_prog, "texture2" ), 1 ); // set texture2 to texture unit 1

    glEnable( GL_DEPTH_TEST );

    Camera camera;
    auto i = camera.getProjectionMatrix();

    // Render loop
    while( !glfwWindowShouldClose( window ) )
    {
        #define RENDER_BLOCK_QUIET
        #ifndef RENDER_BLOCK_QUIET
        utils::time_block( // Time the whole render block
            [&]()
            {
        #endif
                // Deterministic calculations before
                float t   = glfwGetTime();
                float dt  = t - lastFrame;
                lastFrame = t;
                glfwGetFramebufferSize( window, &WIDTH, &HEIGHT );
                float ASPECT         = static_cast<float>( WIDTH ) / static_cast<float>( HEIGHT );
                glm::mat4 view       = glm::lookAt( cameraPos, cameraFront + cameraPos, cameraUp );
                glm::mat4 projection = glm::perspective( glm::radians( FOV ), ASPECT, 0.1f, 100.0f );

                // Inputs: Check and call events and swap the buffers
                glfwSwapBuffers( window );
                glfwPollEvents();
                process_input( window, dt );

                // Colors and Depth
                glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

                // Bind multiple textures
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, wall_tex );
                glActiveTexture( GL_TEXTURE1 );
                glBindTexture( GL_TEXTURE_2D, saul_tex );

                // Set dynamically changing uniforms
                glUniform1f( thetaLoc, theta );
                glUniform1f( timeLoc, t );
                glUniform1f( mix_paramLoc, mix_param );
                glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
                glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );

                // Draw boxes
                glBindVertexArray( vao );
                for( size_t i{ cubePositions.size() }; i-- > 0; )
                {
                    glm::mat4 model = glm::translate( I4, cubePositions[i] );
                    float angle     = 20.0f * i; // Provide a random angle
                    model           = glm::rotate( model, glm::radians( angle ), glm::vec3( 1.0f, 0.3f, 0.5f ) );
                    glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
                    glDrawArrays( GL_TRIANGLES, 0, 36 );
                }

                #ifdef __APPLE__
                glFinish(); // optional to synchronize draw calls. Reduces stuttering on OSX
                #endif

                // Finish using our program. Only necessary if we are using multiple programs
                // glUseProgram(0);
        #ifndef RENDER_BLOCK_QUIET
            },
            "Render Block" );
        #endif
    }

    // de-allocate all resources once they've outlived their purpose
    const GLuint textures[] = { wall_tex, saul_tex };
    glDeleteTextures( 2, textures );
    glDeleteVertexArrays( 1, &vao );
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ebo );
    glDeleteProgram( shader_prog );
    glfwDestroyWindow( window );

    glfwTerminate();

    log_info( "Window terminated successfully" );

    return EXIT_SUCCESS;
}
