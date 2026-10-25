#include "engine/texture.hpp"

#include "engine/runtime.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern "C"
{
#include <log.h>
}

namespace engine
{
    void gen_texture( GLuint& texture, const std::string& path )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        int width, height, nrChannels;
        const std::string& texture_path = get_runtime_dir().append( "textures" ).append( path ).string();
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
} // namespace engine
