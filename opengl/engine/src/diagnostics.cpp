#include "engine/diagnostics.hpp"

#include <glad/glad.h>

extern "C"
{
#include <log.h>
}

namespace engine
{
    void display_device_info()
    {
        log_info( "Loaded OpenGL: %s", glGetString( GL_VERSION ) );
        log_info( "Graphics Device: %s", glGetString( GL_RENDERER ) );
        log_info( "Vendor: %s", glGetString( GL_VENDOR ) );
        log_info( "Shading Language Version: %s", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }
} // namespace engine
