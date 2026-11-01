#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec2 texture;
};

class Terrain
{
  public:
    explicit Terrain( unsigned int meshCells, unsigned int heightMapResolution, float size );
    ~Terrain() = default;

    [[nodiscard]] const float getSize() const;
    [[nodiscard]] const unsigned int getResolution() const;
    [[nodiscard]] const unsigned int getMeshCells() const;
    [[nodiscard]] const std::vector<TerrainVertex>& getVertices() const;
    [[nodiscard]] const std::vector<unsigned int>& getIndices() const;
    [[nodiscard]] const std::vector<float>& getHeights() const;
    [[nodiscard]] const float getElevationAt( float x, float z );
    [[nodiscard]] const glm::vec2 getMinXZ() const;

    [[nodiscard]] constexpr size_t getSizeOfVertexType() const { return sizeof( decltype( m_verticies )::value_type ); }

    [[nodiscard]] constexpr size_t getSizeOfIndicesType() const { return sizeof( decltype( m_indicies )::value_type ); }

    [[nodiscard]] const size_t getVerticiesSize() const;
    [[nodiscard]] const size_t getIndicesSize() const;
    [[nodiscard]] const size_t getArrayBufferWidth() const;
    [[nodiscard]] const size_t getElementBufferWidth() const;

    [[nodiscard]] constexpr void* getOffsetOfVertexPosition() const
    {
        return reinterpret_cast<void*>( offsetof( decltype( m_verticies )::value_type, position ) );
    }

    [[nodiscard]] constexpr void* getOffsetOfVertexTexture() const
    {
        return reinterpret_cast<void*>( offsetof( decltype( m_verticies )::value_type, texture ) );
    }


  private:
    unsigned int m_meshCells;
    unsigned int m_heightMapResolution;
    float m_size;
    float m_spacing;

    std::vector<TerrainVertex> m_verticies;
    std::vector<unsigned int> m_indicies;
    std::vector<float> m_heights;
};

#endif // TERRAIN_HPP
