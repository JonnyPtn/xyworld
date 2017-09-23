#pragma once

#include <array>

#include "FastNoise.h"

constexpr int ChunkSize(128); 
constexpr int TileCount(ChunkSize*ChunkSize);
constexpr int TileSize(4.f); // tile size in pixels

struct TerrainChunk
{
    TerrainChunk() :
        data{ {0u} }
    {};

    void generate(FastNoise& noise, sf::Vector2i index)
    {
        m_index = index;
        for (int x(0); x < ChunkSize; x++)
        {
            for (int y(0); y < ChunkSize; y++)
            {
                auto n = noise.GetSimplexFractal(index.x * ChunkSize + x, index.y * ChunkSize +  y);

                // Some random values for different tile types
                if ( n > 0.5 ) 
                    data[x * ChunkSize + y] = 3;
                else if ( n > 0.2 )
                    data[x * ChunkSize + y] = 2;
                else if (n > 0)
                    data[x * ChunkSize + y] = 1;
            }
        }   
    }

    sf::Vector2i getIndex() const { return m_index; };

    std::array<std::uint16_t, TileCount> data;

private:
    sf::Vector2i m_index;
};