#pragma once

#include <array>

#include "FastNoise.h"

constexpr int ChunkSize(128); 
constexpr int TileCount(ChunkSize*ChunkSize);
constexpr int TileSize(16.f); // tile size in pixels
constexpr float SeaLevel(0.f); // range -1.0 to 1.0

struct TerrainData
{
    float height;
};

struct TerrainChunk
{
    TerrainChunk() :
        data{ {0u} }
    {};

    void generate(FastNoise& noise, sf::Vector2i index)
    {
        m_index = index;
        for (int y(0); y < ChunkSize; y++)
        {
            for (int x(0); x < ChunkSize; x++)
            {
                auto n = noise.GetSimplexFractal(index.x * ChunkSize + x, index.y * ChunkSize +  y);

                data[y * ChunkSize + x].height = n;
            }
        }   
    }

    sf::Vector2i getIndex() const { return m_index; };

    std::array<TerrainData, TileCount> data;

private:
    sf::Vector2i m_index;
};