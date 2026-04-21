#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#define HEIGHT 800
#define WIDTH  1200
#define TWOPI  6.28f
#define PI     3.14159f

#define TIME_BLOCK_QUIET 1

extern "C"
{
    #include <log.h>
}

namespace
{
    static inline void startup_info();
    static inline void process_input( GLFWwindow* window );
    static inline void framebuffer_size_callback( GLFWwindow* window, int width, int height );
    static inline GLuint gen_shaders( const GLenum type, const GLsizei count, const char* const* shaderSource );
    static inline GLuint gen_shader_program( const std::vector<GLuint>& shaders );
    static inline bool shader_compile_check( const GLuint shader );
    static inline bool shader_link_check( const GLuint shaderProgram );
    static inline void time_block( std::function<void()> pred, const char* name = "Unknown" );

    float dX        = 0.0;
    float dY        = 0.0;
    float press_dur = 0.0;
    float theta     = 0.00;

    static inline std::filesystem::path get_runtime_dir()
    {
#ifdef __APPLE__
        char buffer[1024];
        uint32_t size = sizeof( buffer );
        if( _NSGetExecutablePath( buffer, &size ) == 0 )
        {
            std::filesystem::path exec_path   = std::filesystem::path( buffer );
            std::filesystem::path runtime_dir = exec_path.parent_path().parent_path(); // exec_path include exec name

            log_debug( "Found runtime path: %s", runtime_dir.string().c_str() );

            return runtime_dir;
        }
        else
        {
            log_error( "Could not find current executable path." );
            return "";
        }
#endif
        log_error( "TODO" );
        return "";
    }

    static inline std::string load_shader_source( const std::string fileName )
    {
        std::string shader_path = get_runtime_dir().append( "shader" ).append( fileName );
        std::ifstream file( shader_path, std::ios::in );

        if( !file )
        {
            log_error( "Failed opening file: %s", shader_path.c_str() );
            return "";
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static inline std::string load_texture_source( const std::string fileName )
    {
        std::string shader_path = get_runtime_dir().append( "texture/" ).append( fileName );
        std::ifstream file( shader_path, std::ios::in );

        if( !file )
        {
            log_error( "Failed opening file: %s", shader_path.c_str() );
            return "";
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static inline void startup_info()
    {
        log_info( "Loaded OpenGL: \t\t%s", glGetString( GL_VERSION ) );
        log_info( "Graphics Device: \t\t%s", glGetString( GL_RENDERER ) );
        log_info( "Vendor: \t\t\t%s", glGetString( GL_VENDOR ) );
        log_info( "Shading Language Version: \t%s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }

    static inline void process_input( GLFWwindow* window )
    {
        if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        {
            glfwSetWindowShouldClose( window, true );
        }

        if( glfwGetKey( window, GLFW_KEY_SEMICOLON ) == GLFW_PRESS )
        {
            GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode( monitor );
            glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
            log_info( "Set window to fullscreen and changed refersh rate." );
        }

        if( glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS )
        {
            dY += 0.05;
        }
        else if( glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS )
        {
            dY += -0.05;
        }

        if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
        {
            dX += -0.05;
        }
        else if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
        {
            dX += 0.05;
        }

        if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        {
            theta += -0.05 - press_dur;
            press_dur += 0.001;
        }
        else if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        {
            theta += 0.05 + press_dur;
            press_dur += 0.001;
        }
        else if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_RELEASE )
        {
            press_dur = 0.0;
        }
        else if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_RELEASE )
        {
            press_dur = 0.0;
        }
    }

    static inline void framebuffer_size_callback( GLFWwindow* window, int width, int height )
    {
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport( 0, 0, width, height );
    }

    static inline GLuint gen_shaders( const GLenum type, const GLsizei count, const char* const* shaderSource )
    {
        GLuint shader = glCreateShader( type );
        glShaderSource( shader, count, shaderSource, NULL );
        glCompileShader( shader );

        if( !shader_compile_check( shader ) )
            return 0;

        return shader;
    }

    static inline GLuint gen_shader_program( const std::vector<GLuint>& shaders )
    {
        GLuint program = glCreateProgram();

        for( const GLuint& shader : shaders )
            glAttachShader( program, shader ); // Delete shader objects after linking

        glLinkProgram( program );

        for( const GLuint& shader : shaders )
            glDeleteShader( shader ); // Delete shader objects after linking

        if( !shader_link_check( program ) )
            return 0;

        return program;
    }

    static inline bool shader_compile_check( const GLuint shader )
    {
        int success;
        char infoLog[512];
        glGetShaderiv( shader, GL_COMPILE_STATUS, &success );

        if( !success )
        {
            glGetShaderInfoLog( shader, sizeof( infoLog ), NULL, infoLog );
            log_error( "SHADER::COMPILATION_FAILED: %s", infoLog );
        }

        return success;
    }

    static inline bool shader_link_check( const GLuint shaderProgram )
    {
        int success;
        char infoLog[512];
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );

        if( !success )
        {
            glGetProgramInfoLog( shaderProgram, sizeof( infoLog ), NULL, infoLog );
            log_error( "SHADER::LINK_FAILED: %s", infoLog );
        }

        return success;
    }

    static inline void time_block( std::function<void()> pred, const char* name )
    {
#if !TIME_BLOCK_QUIET
        auto start = std::chrono::steady_clock::now();
#endif
        pred();
#if !TIME_BLOCK_QUIET
        auto end      = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end - start );
        log_trace( "%s - Time taken: %lld ms", name, duration.count() );
#endif
    }
} // namespace

int main()
{
    setup_logger( NULL, LOG_DEBUG, true );

    if( !glfwInit() )
    {
        return EXIT_FAILURE;
    }

    // macOS specific hints for modern OpenGL
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
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    if( glewInit() != GLEW_OK )
    {
        log_error( "GLEW Init Failed" );
        return EXIT_FAILURE;
    }

    startup_info();

    // Preparation for using shaders
    const std::string vertex_source   = load_shader_source( "vertex.vert" );
    const std::string fragment_source = load_shader_source( "fragment.frag" );

    const char* vertex_source_ptr   = vertex_source.c_str();
    const char* fragment_source_ptr = fragment_source.c_str();

    log_debug( "Found Vertex Shader: %s", vertex_source_ptr );
    log_debug( "Found Vertex Shader: %s", fragment_source_ptr );

    const GLuint vertex_shader   = gen_shaders( GL_VERTEX_SHADER, 1, &vertex_source_ptr );
    const GLuint fragment_shader = gen_shaders( GL_FRAGMENT_SHADER, 1, &fragment_source_ptr );

    const std::vector<GLuint> shaders = { vertex_shader, fragment_shader }; // Must be in order
    const GLuint shader_prog          = gen_shader_program( shaders );

    glUseProgram( shader_prog );

    // clang-format off
    GLfloat verticies[] = { 
        // positions         // colors          // texture
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,   // bottom left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   0.5f, 1.0f, 0.0f,   // top middle

        -0.5f,  0.5f, 0.0f,     // top left 
        0.0f,  0.5f, 0.0f,      // top middle
        0.0f, -0.5f, 0.0f,      // bottom middle

        0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f
    };

    GLuint indicies[] = {
        0, 1, 2,  // first Triangle
        1, 2, 3,  // second Triangle

        6, 2, 1,
        7, 0, 3
    };
    // clang-format on

    GLuint vao, vbo, ebo;
    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glGenBuffers( 1, &ebo );

    // Original Drawing
    // Bind the vertex array object first, then bind and set vertex buffer(s), and then configure vertex attributes.
    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( verticies ), &verticies, GL_STATIC_DRAW );

    // position attribute
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof( float ), (void*)0 );
    glEnableVertexAttribArray( 0 );
    // color attribute
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof( float ), (void*)( 3 * sizeof( float ) ) );
    glEnableVertexAttribArray( 1 );
    // texture attribute
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof( float ), (void*)( 6 * sizeof( float ) ) );
    glEnableVertexAttribArray( 2 );

    float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );


    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indicies ), &indicies, GL_STATIC_DRAW );

    // We can unbind the VBO and VAO. Unbinding the VAO isn't strictly necessary.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    while( !glfwWindowShouldClose( window ) ) // Render loop
    {
        // Inputs
        process_input( window );

        // Rendering
        glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        time_block(
            [&]()
            {
                float t            = glfwGetTime();
                int offsetLocation = glGetUniformLocation( shader_prog, "offset" );
                int thetaLocation  = glGetUniformLocation( shader_prog, "theta" );
                int timeLocation   = glGetUniformLocation( shader_prog, "t" );
                glUniform2f( offsetLocation, dX, dY );
                glUniform1f( thetaLocation, theta );
                glUniform1f( timeLocation, t );

                glBindVertexArray( vao );
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0 );
                // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                // glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)( 6 * sizeof( GLuint ) ) );
            },
            "Draw Block" );

        // Check and call events and swap the buffers
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glfwTerminate();

    log_info( "Window terminated successfully" );

    return EXIT_SUCCESS;
}