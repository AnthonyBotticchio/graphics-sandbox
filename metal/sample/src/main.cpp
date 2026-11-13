#include "engine/application.hpp"
#include "engine/camera.hpp"
#include "engine/runtime.hpp"
#include "engine/timers.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <SDL3/SDL_main.h>

extern "C"
{
#include <log.h>
}

int main( int, char** )
{
    setup_logger( nullptr, LOG_INFO, false );

    auto pool            = NS::TransferPtr( NS::AutoreleasePool::alloc()->init() );
    auto& app            = engine::Application::getInstance();
    SDL_AppResult result = SDL_APP_FAILURE;

    try
    {
        if( !app.initialize( 900, 600 ) )
            return 1;

        // We are using SDL for the windowing, SDL provides an SDL_Metal_Layer which is backed by NSView/UIView.
        // Our application class will own this layer and SDL functions, however, we need to create a device using this layer
        // as SDL doesn't do that automatically unless we want to use their GPU abstractions (we don't)
        CA::MetalLayer* layer = app.metalLayer();
        const auto shaderPath = engine::get_runtime_dir() / "shader" / "triangle.metal";
        auto device           = NS::TransferPtr( MTL::CreateSystemDefaultDevice() );
        if( !device )
            throw std::runtime_error( "No Metal device available" );

        auto queue = NS::TransferPtr( device->newCommandQueue() );
        if( !queue )
            throw std::runtime_error( "Failed to create Metal command queue" );

        std::ifstream file( shaderPath );
        if( !file )
        {
            throw std::runtime_error( "Failed to read shader: " + shaderPath.string() );
        }
        const std::string shaderSource{ std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() };

        NS::Error* error = nullptr;
        auto library =
            NS::TransferPtr( device->newLibrary( NS::String::string( shaderSource.c_str(), NS::UTF8StringEncoding ), nullptr, &error ) );

        if( !library )
            throw std::runtime_error( std::string( "Failed to compile shader: " ) +
                                      ( error ? error->localizedDescription()->utf8String() : "Unknown error" ) );

        auto vertexFn   = NS::TransferPtr( library->newFunction( MTLSTR( "vertexMain" ) ) );
        auto fragmentFn = NS::TransferPtr( library->newFunction( MTLSTR( "fragmentMain" ) ) );

        if( !vertexFn || !fragmentFn )
            throw std::runtime_error( "Shader must define vertexMain and fragmentMain" );

        auto descriptor = NS::TransferPtr( MTL::RenderPipelineDescriptor::alloc()->init() );
        descriptor->setVertexFunction( vertexFn.get() );
        descriptor->setFragmentFunction( fragmentFn.get() );
        descriptor->colorAttachments()->object( 0 )->setPixelFormat( MTL::PixelFormatBGRA8Unorm );
        auto pipeline = NS::TransferPtr( device->newRenderPipelineState( descriptor.get(), &error ) );

        if( !pipeline )
            throw std::runtime_error( std::string( "Failed to create render pipeline: " ) +
                                      ( error ? error->localizedDescription()->utf8String() : "Unknown error" ) );
        layer->setDevice( device.get() );
        layer->setPixelFormat( MTL::PixelFormatBGRA8Unorm );
        layer->setFramebufferOnly( true );
        layer->setDisplaySyncEnabled( true );
        log_info( "Metal Device: %s", device->name()->utf8String() );

        engine::Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );

        const auto mbtnDown = app.addMouseButtonDownCallback( []( const SDL_MouseButtonEvent& event )
                                                              { log_trace( "Mouse Button Down - x: %f, y: %f", event.x, event.y ); } );

        result = SDL_APP_CONTINUE;
        while( result == SDL_APP_CONTINUE )
        {
            // ENGINE_SCOPED_TIMER( "Render Frame" )

            SDL_Event event;
            while( SDL_PollEvent( &event ) )
            {
                // ENGINE_SCOPED_TIMER( "Handle Events" )
                result = app.handleEvent( &event );
                if( result != SDL_APP_CONTINUE )
                    break;
            }

            if( result != SDL_APP_CONTINUE )
                break;

            result = app.iterate();

            if( result != SDL_APP_CONTINUE )
                break;

            const int width  = app.drawableWidth();
            const int height = app.drawableHeight();
            if( width <= 0 || height <= 0 )
                continue;

            // Begin Render

            // Factory-created Metal objects below live until the end of this frame.
            auto pool               = NS::TransferPtr( NS::AutoreleasePool::alloc()->init() );
            const auto drawableSize = layer->drawableSize();
            if( drawableSize.width != width || drawableSize.height != height )
                layer->setDrawableSize( CGSizeMake( width, height ) );

            auto* drawable = layer->nextDrawable();
            if( !drawable )
                continue;

            auto* commands = queue->commandBuffer();
            if( !commands )
            {
                throw std::runtime_error( "Failed to create Metal command buffer" );
            }

            auto* pass  = MTL::RenderPassDescriptor::renderPassDescriptor();
            auto* color = pass->colorAttachments()->object( 0 );
            color->setTexture( drawable->texture() );
            color->setLoadAction( MTL::LoadActionClear );
            color->setStoreAction( MTL::StoreActionStore );
            color->setClearColor( MTL::ClearColor( 0.08, 0.08, 0.12, 1.0 ) );

            auto* encoder = commands->renderCommandEncoder( pass );
            if( !encoder )
            {
                throw std::runtime_error( "Failed to create Metal render command encoder" );
            }

            const float t                   = static_cast<float>( SDL_GetTicksNS() / 1e9 );
            const float aspect              = static_cast<float>( width ) / height;
            const glm::mat4& viewProjection = camera.getProjectionMatrix( aspect ) * camera.getViewMatrix();

            auto vertArgEncoder = vertexFn->newArgumentEncoder( 0 );
            auto vertArgBuffer  = device->newBuffer( vertArgEncoder->encodedLength(), MTL::ResourceStorageModeShared );
            vertArgEncoder->setArgumentBuffer( vertArgBuffer, 0 );
            // vertArgEncoder->setBuffer( t, 0, 0 );

            encoder->setRenderPipelineState( pipeline.get() );
            encoder->setVertexBytes( &viewProjection, sizeof( viewProjection ), 0 );
            encoder->setFragmentBytes( &t, sizeof( t ), 0 );
            encoder->setViewport( MTL::Viewport{ 0.0, 0.0, double( width ), double( height ), 0.0, 1.0 } );
            encoder->drawPrimitives( MTL::PrimitiveTypeTriangle, NS::UInteger( 0 ), NS::UInteger( 3 ) );
            encoder->endEncoding();

            // Further render, compute, or blit encoders can use this command buffer
            // after the previous encoder has ended and before committing it.
            commands->presentDrawable( drawable );
            commands->commit();
            // Keep one frame in flight for now; all frame resources finish before cleanup.
            commands->waitUntilCompleted();
            if( commands->status() == MTL::CommandBufferStatusError )
            {
                auto* error = commands->error();
                throw std::runtime_error( std::string( "Metal command buffer failed: " ) +
                                          ( error ? error->localizedDescription()->utf8String() : "Unknown error" ) );
            }
        }
    }
    catch( const std::exception& error )
    {
        log_error( "%s", error.what() );
        result = SDL_APP_FAILURE;
    }

    app.shutdown();
    return result == SDL_APP_SUCCESS ? 0 : 1;
}
