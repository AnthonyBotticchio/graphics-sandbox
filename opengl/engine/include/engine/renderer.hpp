#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP

namespace engine
{
    // Frame/pass setup only. Samples still bind shaders and geometry and issue draws explicitly.
    // TODO: Design shader ownership and draw submission as the renderer grows. Abstract draw calls and create a RenderCommand struct
    class Renderer
    {
      public:
        Renderer()  = default;
        ~Renderer() = default;

        void beginFrame( int width, int height );
        void beginOpaquePass();
        void beginParticlePass();
    };
} // namespace engine

#endif // ENGINE_RENDERER_HPP
