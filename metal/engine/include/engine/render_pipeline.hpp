#ifndef ENGINE_RENDER_PIPELINE_HPP
#define ENGINE_RENDER_PIPELINE_HPP

#include <filesystem>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

class RenderPipeline
{
  public:
    enum class RenderPipelineResult
    {
        RP_Success = 0,
        RP_Failure,
        RP_Unknown,
        RP_InvalidFunctionNames,
        RP_InvalidSource
    };

    RenderPipeline( CA::MetalLayer* layer, MTL::Device* device, MTL::CommandQueue* queue );
    virtual ~RenderPipeline() = default;

    void addShaderSource( const std::filesystem::path* path, const char** names = nullptr );
    void beginRenderPipeline( const NS::String* functionNames, const MTL::CompileOptions* options, NS::Error** error );
    void endRenderPipeline();
    void beginRenderPass();
    void endRenderPass();
    static MTL::CommandBuffer* createCommandBuffer();


  private:
    MTL::Device* m_device                                     = nullptr; // Borrowed
    CA::MetalLayer* m_layer                                   = nullptr; // Borrowed
    NS::SharedPtr<MTL::CommandQueue> m_queue                  = nullptr;
    NS::SharedPtr<MTL::Library> m_library                     = nullptr;
    NS::SharedPtr<MTL::RenderPipelineDescriptor> m_descriptor = nullptr;
    NS::SharedPtr<MTL::RenderPipelineState> m_state           = nullptr;
    std::vector<NS::SharedPtr<MTL::Function>> m_functions;
};

#endif // ENGINE_RENDER_PIPELINE_HPP
