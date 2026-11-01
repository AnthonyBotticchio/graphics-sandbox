#include "terrain.hpp"

Terrain::Terrain( unsigned int meshCells, unsigned int heightMapResolution, float size )
    : m_meshCells( meshCells )
    , m_heightMapResolution( heightMapResolution )
    , m_size( size )
    , m_spacing( size / meshCells )
{
    // Generate verticies
    // for( int z = 0; z <= meshCells; ++z )
    // {
    //     for( int x = 0; x <= meshCells; ++x )
    //     {
    //         const float worldX = x * m_spacing - size * 0.5f;
    //         const float worldZ = z * m_spacing - size * 0.5f;
    //         const auto height  = getElevationAt( worldX, worldZ );
    //
    //         m_verticies.emplace_back( glm::vec3( worldX, height, worldZ ), glm::vec2( worldX, worldZ ) / 4.0f );
    //     }
    // }

    // Generate Indicies
    for( int z = 0; z < meshCells; ++z )
    {
        for( int x = 0; x < meshCells; ++x )
        {
            // For each meshCell we have to generate 4 indicies
            const unsigned int a = z * ( meshCells + 1 ) + x;
            const unsigned int b = a + 1;
            const unsigned int c = a + ( meshCells + 1 );
            const unsigned int d = c + 1;

            m_indicies.insert( std::end( m_indicies ), { a, c, b, b, c, d } );
        }
    }

    // Generate HeightMap
    const float heightMapSpacing = size / ( heightMapResolution - 1 );
    for( int z = 0; z < heightMapResolution; ++z )
    {
        for( int x = 0; x < heightMapResolution; ++x )
        {
            const float worldX = x * heightMapSpacing - size * 0.5f;
            const float worldZ = z * heightMapSpacing - size * 0.5f;
            m_heights.emplace_back( getElevationAt( worldX, worldZ ) );
        }
    }
}

const float Terrain::getSize() const
{
    return m_size;
}

const unsigned int Terrain::getResolution() const
{
    return m_heightMapResolution;
}

const unsigned int Terrain::getMeshCells() const
{
    return m_meshCells;
}

const std::vector<TerrainVertex>& Terrain::getVertices() const
{
    return m_verticies;
}

const std::vector<unsigned int>& Terrain::getIndices() const
{
    return m_indicies;
}

const std::vector<float>& Terrain::getHeights() const
{
    return m_heights;
}

const float Terrain::getElevationAt( float x, float z )
{
    // Bend the ridges along Z.
    float warpedX = x + 18.0f * std::sin( z * 0.012f );

    // Dunes roughly 180 world units apart.
    float phase = warpedX * ( 6.283185f / 180.0f ) * 5.0f;

    // A second harmonic makes the slopes asymmetric.
    float dunes = std::sin( phase ) + 0.25f * std::sin( 2.0f * phase );

    // Slowly vary their height along the ridges.
    float height = 5.0f + 3.0f * std::sin( z * 0.008f );

    return -5.0f + height * dunes;
}

const glm::vec2 Terrain::getMinXZ() const
{
    return glm::vec2( -m_size * 0.5f );
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
