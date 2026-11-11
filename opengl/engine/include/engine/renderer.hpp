#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP

namespace engine
{
    class Window;
}

namespace engine
{
    // Frame/pass setup only. Samples still bind shaders and geometry and issue draws explicitly.
    // TODO: Design shader ownership and draw submission as the renderer grows. Abstract draw calls and create a RenderCommand struct
    class Renderer
    {
      public:
        explicit Renderer( Window* ctx );
        ~Renderer() = default;

        void beginFrame();
        void endFrame();
        void beginOpaquePass();
        void beginParticlePass();
        void endParticlePass();

      private:
        Window* m_window = nullptr;
    };
} // namespace engine

#endif // ENGINE_RENDERER_HPP
