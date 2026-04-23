#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

extern "C"
{
    #include <log.h>
}

#define HEIGHT 800
#define WIDTH  800
#define ASPECT (float)( WIDTH / HEIGHT )

#define TIME_BLOCK_QUIET 1

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

    float dX        = 0.00f;
    float dY        = 0.00f;
    float dZ        = 0.00f;
    float press_dur = 0.00f;
    float theta     = 0.00f;
    float mix_param = 0.00f;

    static inline void apply_projection( const GLuint& modelLoc, const GLuint& viewLoc, const GLuint& projLoc )
    {
        // note that we're translating the scene in the reverse direction of where we want to move
        glm::mat4 model      = glm::rotate( glm::mat4( 1.0f ), glm::radians( -55.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
        model                = glm::rotate( model, (float)glfwGetTime() * glm::radians( 50.0f ), glm::vec3( 0.5f, 1.0f, 0.0f ) );
        glm::mat4 view       = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, -3.0f ) );
        glm::mat4 projection = glm::perspective( glm::radians( 45.0f ), ASPECT, 0.1f, 100.0f );

        glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr( model ) );
        glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
    }

    static inline std::filesystem::path get_runtime_dir()
    {
#ifdef __APPLE__
        char buffer[1024];
        uint32_t size = sizeof( buffer );
        if( _NSGetExecutablePath( buffer, &size ) == 0 )
        {
            std::filesystem::path exec_path   = std::filesystem::path( buffer );
            std::filesystem::path runtime_dir = exec_path.parent_path().parent_path(); // exec_path include exec name
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
        std::string shader_path = get_runtime_dir().append( "shader" ).append( fileName ).string();
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

    static inline std::string get_texture_path( const std::string fileName )
    {
        log_debug( "Returning texture path: %s", get_runtime_dir().append( "textures/" ).append( fileName ).string().c_str() );
        return get_runtime_dir().append( "textures" ).append( fileName ).string();
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
            dY -= 0.05;
        }

        if( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS )
        {
            dX -= 0.05;
        }
        else if( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS )
        {
            dX += 0.05;
        }

        if( glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS )
        {
            dZ += 0.05;
        }
        else if( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS )
        {
            dZ -= 0.05;
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

        if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        {
            mix_param += 0.01;
        }
        else if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        {
            mix_param -= 0.01;
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

    static inline void gen_texture( GLuint& texture, const std::string& path )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        int width, height, nrChannels;
        std::string texture_path = get_texture_path( path );
        unsigned char* data      = stbi_load( texture_path.c_str(), &width, &height, &nrChannels, 0 );
        if( data )
        {
            GLenum format;
            if( nrChannels == 1 )
                format = GL_RED;
            else if( nrChannels == 2 )
                format = GL_RG;
            else if( nrChannels == 3 )
                format = GL_RGB;
            else if( nrChannels == 4 )
                format = GL_RGBA;

            glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data );
            glGenerateMipmap( GL_TEXTURE_2D );
        }
        else
        {
            log_error( "Failed to load texture. Path: %s", texture_path.c_str() );
        }
        stbi_image_free( data );

        // Unbind texture
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    static inline bool shader_compile_check( const GLuint shader )
    {
        int success;
        char infoLog[512];
        glGetShaderiv( shader, GL_COMPILE_STATUS, &success );

        if( !success )
        {
            glGetShaderInfoLog( shader, sizeof( infoLog ), NULL, infoLog );
            log_error( "SHADER::COMPILATION_FAILED:\n%s", infoLog );
        }

        return success;
    }

    static inline bool shader_link_check( const GLuint program )
    {
        int success;
        char infoLog[512];
        glGetProgramiv( program, GL_LINK_STATUS, &success );

        if( !success )
        {
            glGetProgramInfoLog( program, sizeof( infoLog ), NULL, infoLog );
            log_error( "SHADER::LINK_FAILED:\n%s", infoLog );
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
        auto duration = std::chrono::duration<double, std::milli>( end - start ); // Fractional milliseconds
        log_trace( "%s - Time taken: %.3f ms", name, duration.count() );
#endif
    }
} // namespace

int main()
{
    // --- Init ---

    setup_logger( NULL, LOG_DEBUG, true );
    stbi_set_flip_vertically_on_load( true ); // Needed for pngs

    if( !glfwInit() )
    {
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
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    if( glewInit() != GLEW_OK )
    {
        log_error( "GLEW Init Failed" );
        return EXIT_FAILURE;
    }

    startup_info();

    // --- Shaders ---

    const std::string vertex_source   = load_shader_source( "vertex.vert" );
    const std::string fragment_source = load_shader_source( "fragment.frag" );

    const char* vertex_source_ptr   = vertex_source.c_str();
    const char* fragment_source_ptr = fragment_source.c_str();

    log_debug( "Found Vertex Shader:\n%s", vertex_source_ptr );
    log_debug( "Found Fragment Shader:\n%s", fragment_source_ptr );

    const GLuint vertex_shader   = gen_shaders( GL_VERTEX_SHADER, 1, &vertex_source_ptr );
    const GLuint fragment_shader = gen_shaders( GL_FRAGMENT_SHADER, 1, &fragment_source_ptr );

    const std::vector<GLuint> shaders = { vertex_shader, fragment_shader }; // Must be in order
    const GLuint shader_prog          = gen_shader_program( shaders );

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

    constexpr GLfloat vertices[] = {
        // positions         // Texture
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
    GLuint wall_tex, leponge_tex;
    gen_texture( wall_tex, "wall.jpg" );
    gen_texture( leponge_tex, "leponge.jpeg" );

    // Constant Uniforms
    GLuint offsetLocation    = glGetUniformLocation( shader_prog, "offset" );
    GLuint thetaLocation     = glGetUniformLocation( shader_prog, "theta" );
    GLuint timeLocation      = glGetUniformLocation( shader_prog, "t" );
    GLuint mix_paramLocation = glGetUniformLocation( shader_prog, "mix_param" );
    GLuint modelLoc          = glGetUniformLocation( shader_prog, "model" );
    GLuint viewLoc           = glGetUniformLocation( shader_prog, "view" );
    GLuint projLoc           = glGetUniformLocation( shader_prog, "proj" );
    glUniform1i( glGetUniformLocation( shader_prog, "texture1" ), 0 ); // set texture1 to texture unit 0
    glUniform1i( glGetUniformLocation( shader_prog, "texture2" ), 1 ); // set texture2 to texture unit 1
    glUniform1f( glGetUniformLocation( shader_prog, "aspect" ), ASPECT );

    glEnable( GL_DEPTH_TEST );

    // Render loop
    while( !glfwWindowShouldClose( window ) )
    {
        time_block( // Time the whole render block
            [&]()
            {
                // Inputs
                process_input( window );

                // Rendering
                glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

                // Set dynamically changing uniforms
                float t = glfwGetTime();
                glUniform3f( offsetLocation, dX, dY, dZ );
                glUniform1f( thetaLocation, theta );
                glUniform1f( timeLocation, t );
                glUniform1f( mix_paramLocation, mix_param );
                apply_projection( modelLoc, viewLoc, projLoc );

                // Bind multiple textures
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, wall_tex );
                glActiveTexture( GL_TEXTURE1 );
                glBindTexture( GL_TEXTURE_2D, leponge_tex );

                // Bind vertex array
                glBindVertexArray( vao );

                // Draw
                // glPolygonMode( GL_FRONT_AND_BACK, GL_LINES );
                // glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
                glDrawArrays( GL_TRIANGLES, 0, 36 );
                glFinish(); // optional

                // Check and call events and swap the buffers
                glfwSwapBuffers( window );
                glfwPollEvents();
            },
            "Render Block" );
    }

    // optional: de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays( 1, &vao );
    glDeleteBuffers( 1, &vbo );
    glDeleteBuffers( 1, &ebo );
    glDeleteTextures( 1, &wall_tex );
    glDeleteTextures( 1, &leponge_tex );

    glfwTerminate();

    log_info( "Window terminated successfully" );

    return EXIT_SUCCESS;
}