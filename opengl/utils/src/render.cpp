#include "utils/render.hpp"

#include <GL/glew.h>

namespace utils
{
    void beginFrame( int width, int height )
    {
        glViewport( 0, 0, width, height );

        glDisable( GL_SCISSOR_TEST );

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

        glDepthMask( GL_TRUE );

        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClearDepth( 1.0 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    void beginOpaquePass()
    {
        glEnable( GL_DEPTH_TEST );

        glDepthFunc( GL_LESS );
        glDepthMask( GL_TRUE );

        glDisable( GL_BLEND );
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );
        glFrontFace( GL_CCW );
    }

    void beginParticlePass()
    {
        glEnable( GL_DEPTH_TEST );

        glDepthFunc( GL_LESS );
        glDepthMask( GL_FALSE );

        glEnable( GL_BLEND );
        glBlendEquation( GL_FUNC_ADD );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glDisable( GL_CULL_FACE );
    }
} // namespace utils
