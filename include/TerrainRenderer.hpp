#pragma once

#include <array>
#include <unordered_map>
#include <xyginext/ecs/System.hpp>
#include <xyginext/resources/Resource.hpp>

#include "FastNoise.h"
#include "TerrainChunk.hpp"

class TerrainRenderer : public xy::System, public sf::Drawable
{
public:
    TerrainRenderer(xy::MessageBus&);
    void process(float) override;

    // Probably the wrong place for this...
    bool isLand(sf::Vector2f worldPos)
    {
        return m_noise.GetSimplexFractal(worldPos.x / TileSize, worldPos.y / TileSize) > SeaLevel;
    }

private:

    // Use this to cache bounds as it's pretty inefficient calculating
    struct ChunkData
    {
        sf::VertexArray verts;
        sf::FloatRect bounds;
    };

    // One vert array per chunk
    std::unordered_map<xy::Entity::ID,ChunkData> m_drawList;

    void onEntityAdded(xy::Entity) override;
    void onEntityRemoved(xy::Entity) override;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;

    xy::Entity addChunk(sf::Vector2i index);

    FastNoise m_noise;

    sf::Texture* m_sheetTexture;
    xy::TextureResource m_textures;
    xy::Entity m_currentChunk; // The current "center" chunk
};
