#include "utilities/utilities.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "utilities/stb_image.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#endif

extern "C"
{
    #include <log.h>
}

namespace
{
    std::filesystem::path get_runtime_dir()
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
#elif __linux__
        char buffer[1024];
        if( readlink( "/proc/self/exe", buffer, sizeof( buffer ) ) != 0 )
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
#elif __WIN32__
        log_error( "TODO" );
        return std::filesystem::path();
#endif
    }

    std::string get_texture_path( const std::string& fileName )
    {
        std::string texture_path = get_runtime_dir().append( "textures" ).append( fileName ).string();
        log_debug( "Returning texture path: %s", texture_path.c_str() );
        return texture_path;
    }
} // namespace

namespace utilities
{
    std::string load_shader_source( const std::string& fileName )
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

    void startup_info()
    {
        log_info( "Loaded OpenGL: \t\t%s", glGetString( GL_VERSION ) );
        log_info( "Graphics Device: \t\t%s", glGetString( GL_RENDERER ) );
        log_info( "Vendor: \t\t\t%s", glGetString( GL_VENDOR ) );
        log_info( "Shading Language Version: \t%s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }

    void framebuffer_size_callback( GLFWwindow* window, int width, int height )
    {
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport( 0, 0, width, height );
    }

    GLuint gen_shaders( const GLenum type, const GLsizei count, const char* const* shaderSource )
    {
        GLuint shader = glCreateShader( type );
        glShaderSource( shader, count, shaderSource, NULL );
        glCompileShader( shader );

        if( !shader_compile_check( shader ) )
            return 0;

        return shader;
    }

    GLuint gen_shader_program( const std::vector<GLuint>& shaders )
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

    void gen_texture( GLuint& texture, const std::string& path )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP );
        int width, height, nrChannels;
        std::string texture_path = get_texture_path( path );
        stbi_set_flip_vertically_on_load( true );
        unsigned char* data      = stbi_load( texture_path.c_str(), &width, &height, &nrChannels, 0 );
        if( data )
        {
            GLenum format = GL_RGB;
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

    bool shader_compile_check( const GLuint shader )
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

    bool shader_link_check( const GLuint program )
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

    void time_block( std::function<void()> pred, const char* name )
    {
#ifndef TIME_BLOCK_QUIET
        auto start = std::chrono::steady_clock::now();
#endif
        pred();
#ifndef TIME_BLOCK_QUIET
        auto end      = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::milli>( end - start ); // Fractional milliseconds
        log_trace( "%s - Time taken: %.3f ms", name, duration.count() );
#endif
    }
} // namespace utilities
