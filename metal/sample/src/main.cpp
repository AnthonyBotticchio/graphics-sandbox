#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <mach-o/dyld.h>
#include <objc/message.h>

extern "C"
{
#include <log.h>
}

static std::filesystem::path resourceDirectory()
{
    uint32_t size = 0;
    _NSGetExecutablePath( nullptr, &size );
    std::vector<char> path( size );
    _NSGetExecutablePath( path.data(), &size );
    return std::filesystem::canonical( path.data() ).parent_path().parent_path();
}

// metal-cpp does not wrap NSWindow/NSView. Attach the layer through the
// Objective-C runtime's C interface, keeping this translation unit plain C++.
static void attachLayer( GLFWwindow* window, CA::MetalLayer* layer )
{
    id cocoaWindow = glfwGetCocoaWindow( window );
    id view        = reinterpret_cast<id ( * )( id, SEL )>( objc_msgSend )( cocoaWindow, sel_registerName( "contentView" ) );
    reinterpret_cast<void ( * )( id, SEL, BOOL )>( objc_msgSend )( view, sel_registerName( "setWantsLayer:" ), YES );
    reinterpret_cast<void ( * )( id, SEL, CA::MetalLayer* )>( objc_msgSend )( view, sel_registerName( "setLayer:" ), layer );
}

static void render( GLFWwindow* window )
{
    auto device = NS::TransferPtr( MTL::CreateSystemDefaultDevice() );
    if( !device )
        throw std::runtime_error( "No Metal device available" );
    auto queue = NS::TransferPtr( device->newCommandQueue() );
    if( !queue )
        throw std::runtime_error( "Failed to create Metal command queue" );

    const auto shaderPath = resourceDirectory() / "shader" / "triangle.metal";
    std::ifstream file( shaderPath );
    if( !file )
        throw std::runtime_error( "Failed to read shader: " + shaderPath.string() );
    const std::string source{ std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() };
    NS::Error* error = nullptr;
    auto library = NS::TransferPtr( device->newLibrary( NS::String::string( source.c_str(), NS::UTF8StringEncoding ), nullptr, &error ) );
    if( !library )
        throw std::runtime_error( error ? error->localizedDescription()->utf8String() : "Failed to compile shader" );

    auto vertex     = NS::TransferPtr( library->newFunction( MTLSTR( "triangleVertex" ) ) );
    auto fragment   = NS::TransferPtr( library->newFunction( MTLSTR( "triangleFragment" ) ) );
    auto descriptor = NS::TransferPtr( MTL::RenderPipelineDescriptor::alloc()->init() );
    descriptor->setVertexFunction( vertex.get() );
    descriptor->setFragmentFunction( fragment.get() );
    descriptor->colorAttachments()->object( 0 )->setPixelFormat( MTL::PixelFormatBGRA8Unorm );
    auto pipeline = NS::TransferPtr( device->newRenderPipelineState( descriptor.get(), &error ) );
    if( !pipeline )
        throw std::runtime_error( error ? error->localizedDescription()->utf8String() : "Failed to create render pipeline" );

    auto layer = NS::RetainPtr( CA::MetalLayer::layer() );
    layer->setDevice( device.get() );
    layer->setPixelFormat( MTL::PixelFormatBGRA8Unorm );
    layer->setFramebufferOnly( true );
    layer->setDisplaySyncEnabled( true );
    attachLayer( window, layer.get() );
    log_info( "Metal device: %s", device->name()->utf8String() );
    log_info( "mtl-sample started" );

    while( !glfwWindowShouldClose( window ) )
    {
        auto framePool = NS::TransferPtr( NS::AutoreleasePool::alloc()->init() );
        glfwPollEvents();
        if( glfwWindowShouldClose( window ) )
            break;
        int width = 0, height = 0;
        glfwGetFramebufferSize( window, &width, &height );
        if( width == 0 || height == 0 || glfwGetWindowAttrib( window, GLFW_ICONIFIED ) )
        {
            glfwWaitEventsTimeout( 0.1 );
            continue;
        }
        layer->setDrawableSize( CGSizeMake( width, height ) );
        auto* drawable = layer->nextDrawable();
        if( !drawable )
            continue;

        auto* pass  = MTL::RenderPassDescriptor::renderPassDescriptor();
        auto* color = pass->colorAttachments()->object( 0 );
        color->setTexture( drawable->texture() );
        color->setLoadAction( MTL::LoadActionClear );
        color->setStoreAction( MTL::StoreActionStore );
        color->setClearColor( MTL::ClearColor( 0.08, 0.08, 0.12, 1.0 ) );
        auto* commands = queue->commandBuffer();
        auto* encoder  = commands->renderCommandEncoder( pass );
        encoder->setRenderPipelineState( pipeline.get() );
        // Preserve the triangle's proportions when the window is resized.
        const double side = std::min( width, height );
        encoder->setViewport( MTL::Viewport{ ( width - side ) / 2.0, ( height - side ) / 2.0, side, side, 0.0, 1.0 } );
        encoder->drawPrimitives( MTL::PrimitiveTypeTriangle, NS::UInteger( 0 ), NS::UInteger( 3 ) );
        encoder->endEncoding();
        commands->presentDrawable( drawable );
        commands->commit();
        // Keep this introductory sample to one frame in flight.
        commands->waitUntilCompleted();
        if( commands->status() == MTL::CommandBufferStatusError )
            throw std::runtime_error( commands->error()->localizedDescription()->utf8String() );
    }
}

int main()
{
    setup_logger( nullptr, LOG_INFO, false );
    auto pool = NS::TransferPtr( NS::AutoreleasePool::alloc()->init() );
    glfwSetErrorCallback( []( int code, const char* message ) { log_error( "GLFW %d: %s", code, message ); } );
    if( !glfwInit() )
        return 1;

    int result = 0;
    {
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
        std::unique_ptr<GLFWwindow, decltype( &glfwDestroyWindow )> window(
            glfwCreateWindow( 800, 600, "Metal Triangle", nullptr, nullptr ), glfwDestroyWindow );
        if( !window )
            result = 1;
        else
        {
            try
            {
                render( window.get() );
            }
            catch( const std::exception& error )
            {
                log_error( "%s", error.what() );
                result = 1;
            }
        }
    }
    glfwTerminate();
    return result;
}
