#include "engine/runtime.hpp"

#include <cstdint>
#include <string>

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

#ifdef __linux__
    #include <unistd.h>
#endif

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

extern "C"
{
#include <log.h>
}

namespace engine
{
    std::filesystem::path get_runtime_dir()
    {
#ifdef __APPLE__
        char buffer[1024];
        uint32_t size = sizeof( buffer );
        if( _NSGetExecutablePath( buffer, &size ) == 0 )
        {
            std::filesystem::path exec_path   = std::filesystem::path( buffer );
            std::filesystem::path runtime_dir = exec_path.parent_path().parent_path(); // exec_path include exec name
            return runtime_dir;
        }
        else
        {
            log_error( "Could not find current executable path." );
            return "";
        }
#endif

#ifdef __linux__
        char buffer[1024];
        const auto size = readlink( "/proc/self/exe", buffer, sizeof( buffer ) );
        if( size > 0 && static_cast<size_t>( size ) < sizeof( buffer ) )
        {
            std::filesystem::path exec_path   = std::filesystem::path( std::string( buffer, static_cast<size_t>( size ) ) );
            std::filesystem::path runtime_dir = exec_path.parent_path().parent_path(); // exec_path include exec name
            return runtime_dir;
        }
        else
        {
            log_error( "Could not find current executable path." );
            return "";
        }
#endif

#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD size = GetModuleFileNameW( nullptr, buffer, MAX_PATH );

        if( size != 0 && size < MAX_PATH )
        {
            std::filesystem::path exec_path   = std::filesystem::path( buffer );
            std::filesystem::path runtime_dir = exec_path.parent_path().parent_path(); // exec_path include exec name
            return runtime_dir;
        }
        else
        {
            log_error( "Could not find current executable path." );
            return "";
        }
#endif
        log_error( "Unsupported platform." );
        return "";
    }
} // namespace engine
