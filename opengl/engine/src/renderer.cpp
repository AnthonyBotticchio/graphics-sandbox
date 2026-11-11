#include "engine/renderer.hpp"

#include "engine/window.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace engine
{
    Renderer::Renderer( Window* ctx ) : m_window( ctx )
    {
    }

    void Renderer::beginFrame()
    {
        int fbW, fbH;
        m_window->getFramebufferSize( &fbW, &fbH );

        glViewport( 0, 0, fbW, fbH );

        glDisable( GL_SCISSOR_TEST );

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

        glDepthMask( GL_TRUE );

        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClearDepth( 1.0 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    void Renderer::endFrame()
    {
        m_window->swapBuffers();
    }

    void Renderer::beginOpaquePass()
    {
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LESS );
        glDepthMask( GL_TRUE );

        glDisable( GL_BLEND );
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );
        glFrontFace( GL_CCW );
    }

    void Renderer::beginParticlePass()
    {
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LESS );
        glDepthMask( GL_TRUE );

        glEnable( GL_BLEND );
        glBlendEquation( GL_FUNC_ADD );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glDisable( GL_CULL_FACE );
    }

    void Renderer::endParticlePass()
    {
    }

} // namespace engine
