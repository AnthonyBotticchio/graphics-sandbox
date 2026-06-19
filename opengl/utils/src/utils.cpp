#include "utils/utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <chrono>

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

#ifdef __linux__
    #include <unistd.h>
#endif

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

extern "C"
{
#include <log.h>
}

namespace utils
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
#endif

#ifdef __linux__
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
#endif

#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD size = GetModuleFileNameW( nullptr, buffer, MAX_PATH );

        if( size != 0 && size < MAX_PATH )
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
        log_error( "Unsupported platform." );
        return "";
    }

    std::string get_texture_path( const std::string& fileName )
    {
        std::string texture_path = get_runtime_dir().append( "textures" ).append( fileName ).string();
        return texture_path;
    }

    void display_device_info()
    {
        log_info( "Loaded OpenGL: %s", glGetString( GL_VERSION ) );
        log_info( "Graphics Device: %s", glGetString( GL_RENDERER ) );
        log_info( "Vendor: %s", glGetString( GL_VENDOR ) );
        log_info( "Shading Language Version: %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }

    void framebuffer_size_callback( GLFWwindow* window, int width, int height )
    {
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport( 0, 0, width, height );
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
        unsigned char* data = stbi_load( texture_path.c_str(), &width, &height, &nrChannels, 0 );
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

    void time_block( std::function<void()> pred, const char* name )
    {
// #define TIME_BLOCK_QUIET
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
} // namespace utils
