/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct
{
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    void* udata;
    int line;
    int level;
} log_Event;

typedef void ( *log_LogFn )( log_Event* ev );
typedef void ( *log_LockFn )( bool lock, void* udata );

enum
{
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

#define log_trace( ... ) log_log( LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__ )
#define log_debug( ... ) log_log( LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__ )
#define log_info( ... ) log_log( LOG_INFO, __FILE__, __LINE__, __VA_ARGS__ )
#define log_warn( ... ) log_log( LOG_WARN, __FILE__, __LINE__, __VA_ARGS__ )
#define log_error( ... ) log_log( LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__ )
#define log_fatal( ... ) log_log( LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__ )

const char* log_level_string( int level );
void log_set_lock( log_LockFn fn, void* udata );
void log_set_level( int level );
void log_set_quiet( bool enable );
int log_add_callback( log_LogFn fn, void* udata, int level );
int log_add_fp( FILE* fp, int level );

/// @brief Takes away the prepending file path from a string.
/// @param file reference to the char * to the file string.
void strip_filename( const char** file ); // ADDED FUNCTION

void log_log( int level, const char* file, int line, const char* fmt, ... );

// ------- Added Utility Functions -------

/// @brief Sets up the logger to a log path and a level and with logging to STDOUT with a boolean.
/// @param log_path The file path to the log file. Set to NULL for no path.
/// @param level The minimum log level.
/// @param set_quiet If true, logging to STDOUT is disabled. Set to false for console logging.
/// @return true if logger setup was successful, false otherwise.
bool setup_logger( const char* log_path, int level, bool set_quiet );

/// @brief Will allow the logging to set to quiet for this specific level
/// @param level Logging level to set quiet
/// @param set_quiet Setting the logging level's quiet property
void log_set_quiet_level( int level, bool set_quiet );

#endif