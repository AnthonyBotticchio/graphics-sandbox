#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <unordered_map>

#include <GL/glew.h>

class Shader
{
  public:
    Shader( const char* vertexPath, const char* fragmentPath );
    virtual ~Shader() = default;

    const GLuint getProgram() const;

    void use() const;

    template<std::integral T>
    void setUniform( const std::string& name, T value ) const
    {
        glUniform1i( glGetUniformLocation( m_program, name.c_str() ), value );
    }

    template<std::floating_point T>
    void setUniform( const std::string& name, T value ) const
    {
        glUniform1f( glGetUniformLocation( m_program, name.c_str() ), value );
    }

    void setUniform( const std::string& name, const glm::vec2& value ) const;
    void setUniform( const std::string& name, float x, float y ) const;
    void setUniform( const std::string& name, const glm::vec3& value ) const;
    void setUniform( const std::string& name, float x, float y, float z ) const;
    void setUniform( const std::string& name, const glm::vec4& value ) const;
    void setUniform( const std::string& name, float x, float y, float z, float w ) const;
    void setUniform( const std::string& name, const glm::mat2& mat ) const;
    void setUniform( const std::string& name, const glm::mat3& mat ) const;
    void setUniform( const std::string& name, const glm::mat4& mat ) const;

  private:
    /**
     * @brief Returns the cached location for a uniform, querying OpenGL on the first lookup.
     *
     * Uniform locations are cached per shader program to avoid repeated driver string lookups in
     * the render loop. A value of -1 is valid cache data and means the uniform was not found or was
     * optimized out by the shader compiler.
     *
     * Improves uniform lookup performance.
     *
     * @param name Uniform variable name in the linked shader program.
     * @return Uniform location (or -1 if the uniform is inactive or missing)
     */
    GLint uniformCacheLookup( const std::string& name ) const;

    std::string loadShaderSource( const char* fileName );
    GLuint createShader( const GLenum type, const GLsizei count, const char* const* shaderSource );
    bool shaderCompileCheck( const GLuint shader );
    bool shaderLinkCheck( const GLuint program );
    GLuint createShaderProgram( const std::vector<GLuint>& shaders );

    mutable std::unordered_map<std::string, GLint> m_uniformCache;
    GLuint m_program;
};

#endif // SHADER_HPP
