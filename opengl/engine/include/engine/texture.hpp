#ifndef ENGINE_TEXTURE_HPP
#define ENGINE_TEXTURE_HPP

#include <string>

#include <glad/glad.h>

namespace engine
{
    // Loads from the runtime textures/ directory. The caller owns the resulting GL texture.
    void gen_texture( GLuint& texture, const std::string& path );
    // Loads a height mapped terrain texture at a specified resolution
    void gen_terrain_texture( GLuint& texture, unsigned int resolution, const void* data );
} // namespace engine

#endif // ENGINE_TEXTURE_HPP
