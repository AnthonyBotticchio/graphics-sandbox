#include "shader.hpp"

#include "utils/timers.hpp"
#include "utils/utils.hpp"

#include <fstream>

Shader::Shader( const char* vertexPath, const char* fragmentPath, const std::vector<const char*>& feedbackOutputs )
{
    const std::string vertexSource = loadShaderSource( vertexPath );
    const char* vertexText         = vertexSource.c_str();

    std::vector<GLuint> shaders{ createShader( GL_VERTEX_SHADER, 1, &vertexText ) };

    if( fragmentPath != nullptr )
    {
        const std::string fragmentSource = loadShaderSource( fragmentPath );
        const char* fragmentText         = fragmentSource.c_str();

        shaders.push_back( createShader( GL_FRAGMENT_SHADER, 1, &fragmentText ) );
    }

    m_program = createShaderProgram( shaders, feedbackOutputs );
}

GLuint Shader::getProgram() const
{
    return m_program;
}

void Shader::use() const
{
    glUseProgram( m_program );
}

void Shader::setUniform( const std::string& name, const glm::vec2& value ) const
{
    glUniform2fv( uniformCacheLookup( name ), 1, &value[0] );
}

void Shader::setUniform( const std::string& name, float x, float y ) const
{
    glUniform2f( uniformCacheLookup( name ), x, y );
}

void Shader::setUniform( const std::string& name, const glm::vec3& value ) const
{
    glUniform3fv( uniformCacheLookup( name ), 1, &value[0] );
}

void Shader::setUniform( const std::string& name, float x, float y, float z ) const
{
    glUniform3f( uniformCacheLookup( name ), x, y, z );
}

void Shader::setUniform( const std::string& name, const glm::vec4& value ) const
{
    glUniform4fv( uniformCacheLookup( name ), 1, &value[0] );
}

void Shader::setUniform( const std::string& name, float x, float y, float z, float w ) const
{
    glUniform4f( uniformCacheLookup( name ), x, y, z, w );
}

void Shader::setUniform( const std::string& name, const glm::mat2& mat ) const
{
    glUniformMatrix2fv( uniformCacheLookup( name ), 1, GL_FALSE, glm::value_ptr( mat ) );
}

void Shader::setUniform( const std::string& name, const glm::mat3& mat ) const
{
    glUniformMatrix3fv( uniformCacheLookup( name ), 1, GL_FALSE, glm::value_ptr( mat ) );
}

void Shader::setUniform( const std::string& name, const glm::mat4& mat ) const
{
    glUniformMatrix4fv( uniformCacheLookup( name ), 1, GL_FALSE, glm::value_ptr( mat ) );
}

GLint Shader::uniformCacheLookup( const std::string& name ) const
{
    if( auto iter = m_uniformCache.find( name ); iter != std::end( m_uniformCache ) )
    {
        return iter->second;
    }

    GLint location = glGetUniformLocation( m_program, name.c_str() );
    m_uniformCache.emplace( name, location ); // Add location to cache
    return location;
}

std::string Shader::loadShaderSource( const char* fileName )
{
    std::string shader_path = utils::get_runtime_dir().append( "shader" ).append( fileName ).string();
    std::ifstream file( shader_path, std::ios::in );

    if( !file )
    {
        log_error( "Failed opening file: %s", shader_path.c_str() );
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::createShader( const GLenum type, const GLsizei count, const char* const* shaderSource )
{
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, count, shaderSource, NULL );
    glCompileShader( shader );

    if( !shaderCompileCheck( shader ) )
        return 0;

    return shader;
}

bool Shader::shaderCompileCheck( const GLuint shader )
{
    int success;
    char infoLog[512];
    glGetShaderiv( shader, GL_COMPILE_STATUS, &success );

    if( !success )
    {
        glGetShaderInfoLog( shader, sizeof( infoLog ), NULL, infoLog );
        log_error( "SHADER::COMPILATION_FAILED:\n%s", infoLog );
    }

    return success;
}

bool Shader::shaderLinkCheck( const GLuint program )
{
    int success;
    char infoLog[512];
    glGetProgramiv( program, GL_LINK_STATUS, &success );

    if( !success )
    {
        glGetProgramInfoLog( program, sizeof( infoLog ), NULL, infoLog );
        log_error( "SHADER::LINK_FAILED:\n%s", infoLog );
    }

    return success;
}

GLuint Shader::createShaderProgram( const std::vector<GLuint>& shaders, const std::vector<const char*>& feedbackOutputs )
{
    GLuint program = glCreateProgram();

    if( !feedbackOutputs.empty() )
    {
        glTransformFeedbackVaryings( program, static_cast<GLsizei>( feedbackOutputs.size() ), feedbackOutputs.data(),
                                     GL_INTERLEAVED_ATTRIBS );
    }

    for( const GLuint& shader : shaders )
        glAttachShader( program, shader ); // Delete shader objects after linking

    glLinkProgram( program );

    for( const GLuint& shader : shaders )
        glDeleteShader( shader ); // Delete shader objects after linking

    if( !shaderLinkCheck( program ) )
        return 0;

    return program;
}
