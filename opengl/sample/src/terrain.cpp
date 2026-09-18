#include "terrain.hpp"

Terrain::Terrain( unsigned int cells, float size ) : m_cells( cells ), m_size( size ), m_spacing( size / cells )
{
    // Generate verticies
    for( int z = 0; z <= cells; ++z )
    {
        for( int x = 0; x <= cells; ++x )
        {
            float worldX = x * m_spacing - size * 0.5f;
            float worldZ = z * m_spacing - size * 0.5f;

            m_verticies.emplace_back( glm::vec3( worldX, getElevationAt( worldX, worldZ ), worldZ ), glm::vec2( worldX, worldZ ) / 4.0f );
        }
    }

    // Generate Indicies
    for( int z = 0; z < cells; ++z )
    {
        for( int x = 0; x < cells; ++x )
        {
            unsigned int a = z * ( cells + 1 ) + x;
            unsigned int b = a + 1;
            unsigned int c = a + ( cells + 1 );
            unsigned int d = c + 1;

            m_indicies.insert( std::end( m_indicies ), { a, c, b, b, c, d } );
        }
    }
}

const std::vector<TerrainVertex>& Terrain::getVertices() const
{
    return m_verticies;
}

const std::vector<unsigned int>& Terrain::getIndices() const
{
    return m_indicies;
}

const float Terrain::getElevationAt( float x, float z )
{
    // Bend the ridges along Z.
    float warpedX = x + 18.0f * std::sin(z * 0.012f);

    // Dunes roughly 180 world units apart.
    float phase = warpedX * (6.283185f / 180.0f) * 5.0f;

    // A second harmonic makes the slopes asymmetric.
    float dunes = std::sin(phase)
                + 0.25f * std::sin(2.0f * phase);

    // Slowly vary their height along the ridges.
    float height = 5.0f + 3.0f * std::sin(z * 0.008f);

    return -5.0f + height * dunes;
}

const size_t Terrain::getVerticiesSize() const
{
    return m_verticies.size();
}

const size_t Terrain::getIndicesSize() const
{
    return m_indicies.size();
}

const size_t Terrain::getArrayBufferWidth() const
{
    return getSizeOfVertexType() * getVerticiesSize();
}

const size_t Terrain::getElementBufferWidth() const
{
    return getSizeOfIndicesType() * getIndicesSize();
}
