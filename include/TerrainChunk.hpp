#pragma once

#include <array>

#include "FastNoise.h"

constexpr int ChunkSize(64); 
constexpr int TileCount(ChunkSize*ChunkSize);
constexpr int TileSize(16.f); // tile size in pixels

struct TerrainChunk
{
    TerrainChunk() :
        data{ {0u} }
    {};

    void generate(FastNoise& noise)
    {
        for (int x(0); x < ChunkSize; x++)
        {
            for (int y(0); y < ChunkSize; y++)
            {
                auto n = noise.GetSimplexFractal(pos.x + x, pos.y +  y);

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
    std::array<std::uint16_t, TileCount> data;
    sf::Vector2i pos;
};