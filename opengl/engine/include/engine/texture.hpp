#ifndef ENGINE_TEXTURE_HPP
#define ENGINE_TEXTURE_HPP

#include <string>

#include <glad/glad.h>

namespace engine
{
    // Loads from the runtime textures/ directory. The caller owns the resulting GL texture.
    void gen_texture( GLuint& texture, const std::string& path );
} // namespace engine

#endif // ENGINE_TEXTURE_HPP
