#ifndef UTILS_RENDER_HPP
#define UTILS_RENDER_HPP

namespace utils
{
    // Requires a current OpenGL context. Clears the currently bound framebuffer.
    void beginFrame( int width, int height );
    void beginOpaquePass();
    // Additive blending; tests opaque depth without writing particle depth.
    void beginParticlePass();
} // namespace utils

#endif // UTILS_RENDER_HPP
