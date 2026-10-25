#ifndef FRAME_PACER_COMMON_HPP
#define FRAME_PACER_COMMON_HPP

#ifndef GLFW_INCLUDE_NONE
    #define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>

extern "C"
{
#include <log.h>
}

namespace
{
    bool hasDrawableArea( GLFWwindow* window )
    {
        int width, height;
        glfwGetFramebufferSize( window, &width, &height );
        return glfwGetWindowAttrib( window, GLFW_VISIBLE ) && !glfwGetWindowAttrib( window, GLFW_ICONIFIED ) && width > 0 && height > 0;
    }
} // namespace

#endif // FRAME_PACER_COMMON_HPP
