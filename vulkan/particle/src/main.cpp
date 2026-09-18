#include <stdlib.h>

#include <ev2/asset.h>
#include <ev2/context.h>
#include <ev2/pipeline.h>
#include <glm/glm.hpp>

extern "C"
{
    #include <log.h>
}

namespace
{
    ev2::GfxContext* ctx;
}

int main( int argc, char** argv )
{
    log_info("Particle Render App");

    return 1;
}