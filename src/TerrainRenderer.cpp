
#include "TerrainRenderer.hpp"
#include "TerrainChunk.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Random.hpp>

// Draw distance (radius from camera, in world units)
constexpr float DrawDistance(3500.f);


TerrainRenderer::TerrainRenderer(xy::MessageBus& mb) :
    xy::System(mb, typeid(TerrainRenderer)),
    m_noise(),
    m_currentChunk()
{
    requireComponent<TerrainChunk>();
    requireComponent<xy::Transform>();

    m_noise.SetNoiseType(FastNoise::Cellular);

    m_sheetTexture = &m_textures.get("assets/Roguelike_pack/Spritesheet/roguelikeSheet_transparent.png");
}


void TerrainRenderer::process(float dt)
{
    // Get the camera position
    sf::Vector2f pos(0, 0);
    auto camEnt = getScene()->getActiveCamera();
    pos = camEnt.getComponent<xy::Transform>().getWorldTransform().transformPoint(pos);

    // Get the chunk the camera is on
    xy::Entity chunk;
    for (auto& ent : getEntities())
    {
        auto& rect = m_drawList[ent.getIndex()].bounds;
        if (rect.contains(pos))
        {
            chunk = ent;
            break;
        }
    }

    // If the camera isn't on an existing chunk, add a new one
    if (chunk == xy::Entity())
    {
        int x = pos.x / (ChunkSize * TileSize);
        int y = pos.y / (ChunkSize * TileSize);
        chunk = addChunk({ x,y });
    }

    // If the new chunk doesn't match the last one, update all surrounding chunks
    if (chunk != m_currentChunk)
    {
        // Current chunk changed, make sure all surrounding chunks are present
        m_currentChunk = chunk;
        auto c = m_currentChunk.getComponent<TerrainChunk>().getIndex();

        std::list<sf::Vector2i> surroundingChunks{
            { c.x - 1, c.y - 1 }, // top left
            { c.x    , c.y - 1 }, // top 
            { c.x + 1, c.y - 1 }, // top right
            { c.x - 1, c.y     }, // left
            { c.x + 1, c.y     }, // right
            { c.x - 1, c.y + 1 }, // bottom left
            { c.x    , c.y + 1 }, // bottom
            { c.x + 1, c.y + 1 }, // bottom right
        };

        // Remove any chunks already on the draw list
        for (auto& ent : getEntities())
        {
            auto match = std::find_if(surroundingChunks.begin(), surroundingChunks.end(), [&ent](const sf::Vector2i& i) {
                
                auto index = ent.getComponent<TerrainChunk>().getIndex();
                return (index.x == i.x) && (index.y == i.y);
            });

            if (match != surroundingChunks.end())
            {
                surroundingChunks.erase(match);
            }
        }

        // Add remaining chunks
        for (auto& c : surroundingChunks)
        {
            addChunk(c);
        }

    }

    // If a chunk is outside the draw distance, remove it
    // Based on chunk center position, not it's entirety
    for (auto& ent : getEntities())
    {
        sf::FloatRect bounds = m_drawList[ent.getIndex()].bounds;
        
        sf::Vector2f chunkPos = { bounds.left + bounds.width / 2, bounds.top + bounds.height / 2 };

        if (xy::Util::Vector::length(chunkPos - pos) > DrawDistance)
        {
            getScene()->destroyEntity(ent);
            auto pos = ent.getComponent<TerrainChunk>().getIndex();
            xy::Logger::log("Chunk removed at " + std::to_string(pos.x) + "," + std::to_string(pos.y));
        }
    }

}

void TerrainRenderer::onEntityAdded(xy::Entity ent)
{
    // Gather the tile data from this chunk and create verts for it
    auto chunk = ent.getComponent<TerrainChunk>();
    auto pos = ent.getComponent<xy::Transform>().getPosition();
    auto chunkId = chunk.getIndex();

    sf::VertexArray verts(sf::PrimitiveType::Quads);

    for (int y(0); y < ChunkSize; y++)
    {
        for (int x(0); x < ChunkSize; x++)
        {
                auto i = y * ChunkSize + x;
                sf::Vector2f texPos;

                // Check if it's land tile first
                if (chunk.data[i].height > SeaLevel)
                {
                    // Pick one of the random land tiles
                    const std::vector<sf::Vector2f> landTiles =
                    {
                        {85.f,0.f},
                        {85.f,17.f}
                    };
                    auto selection = xy::Util::Random::value(0, landTiles.size() - 1);
                    texPos = landTiles[selection];
                    sf::Vector2f tileGfxSize(16.f, 16.f);
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize }, texPos }); // top left
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize },{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                }
                // the rest is sea -  check the for boundaries and select the right graphic
                else
                {

                    enum Neighbours
                    {
                        None,
                        TL,
                        T = 1 << 1,
                        TR = 1 << 2,
                        R = 1 << 3,
                        BR = 1 << 4,
                        B = 1 << 5,
                        BL = 1 << 6,
                        L = 1 << 7,
                    };

                    int n = None;

                    // Top left
                    if (i % ChunkSize && i - ChunkSize > 0)
                        n |= chunk.data[i - 1 - ChunkSize].height > SeaLevel ? TL : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x - 1, chunkId.y* ChunkSize + y - 1) > SeaLevel ? TL : 0;

                    // Top
                    if (i - ChunkSize > 0)
                        n |= chunk.data[i - ChunkSize].height > SeaLevel ? T : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x, chunkId.y* ChunkSize + y - 1) > SeaLevel ? T : 0;

                    // Top Right
                    if ((i + 1 - ChunkSize) > 0 && (i + 1 - ChunkSize) % ChunkSize)
                        n |= chunk.data[i + 1 - ChunkSize].height > SeaLevel ? TR : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x + 1, chunkId.y* ChunkSize + y - 1) > SeaLevel ? TR : 0;

                    // Right
                    if (i + 1 < TileCount && (i + 1) % ChunkSize)
                        n |= chunk.data[i + 1].height > SeaLevel ? R : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x + 1, chunkId.y* ChunkSize + y) > SeaLevel ? R : 0;

                    // Bottom Right
                    if ((i + 1) % ChunkSize && i < TileCount - ChunkSize)
                        n |= chunk.data[i + 1 + ChunkSize].height > SeaLevel ? BR : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x + 1, chunkId.y* ChunkSize + y + 1) > SeaLevel ? BR : 0;

                    // Bottom
                    if (i + ChunkSize < TileCount)
                        n |= chunk.data[i + ChunkSize].height > SeaLevel ? B : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x, chunkId.y* ChunkSize + y + 1) > SeaLevel ? B : 0;

                    // Bottom Left
                    if (i + ChunkSize < TileCount &&  i % ChunkSize)
                        n |= chunk.data[i + ChunkSize - 1].height > SeaLevel ? BL : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x - 1, chunkId.y* ChunkSize + y + 1) > SeaLevel ? BL : 0;

                    // Left
                    if (i % ChunkSize)
                        n |= chunk.data[i - 1].height > SeaLevel ? L : 0;
                    else
                        n |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x - 1, chunkId.y* ChunkSize + y) > SeaLevel ? L : 0;

                    if (!n)
                    {
                        // Completely surrounded by sea, pick a random sea tile
                        std::vector<sf::Vector2f> seaTexPos =
                        {
                            {51,17},
                            {0,0},
                            {17,0},
                            {51,68}
                        };
                        auto selection = xy::Util::Random::value(0, seaTexPos.size() - 1);
                        texPos = seaTexPos[selection];
                    }

                    else if ((n & (L | T)) == (L | T))
                        texPos = { 34, 0 };
                    else if ((n & (R | T)) == (R | T))
                        texPos = { 68, 0 };
                    else if ((n & (R | B)) == (R | B))
                        texPos = { 68, 34 };
                    else if ((n & (L | B)) == (L | B))
                        texPos = { 34, 34 };

                    else if (n & L)
                        texPos = { 34, 17 };
                    else if (n & R)
                        texPos = { 68, 17 };
                    else if (n & T)
                        texPos = { 51, 0 };
                    else if (n & B)
                        texPos = { 51, 34 };

                    sf::Vector2f tileGfxSize(16.f, 16.f);
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize }, texPos }); // top left
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize }, { texPos.x + tileGfxSize.x, texPos.y} }); // top right
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                    verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                    // Patch over some bits because we don't have tiles for them
                    //

                    if ((n & (L | T | R)) == (L | T | R))
                    {
                        texPos = { 76,0 };
                        tileGfxSize = { 8,16 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize },{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left
                    }

                    if ((n & (L | T | B)) == (L | T | B))
                    {
                        texPos = { 34,41 };
                        tileGfxSize = { 16,8 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize / 2 }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize / 2 },{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left
                    }


                    // Corner bits
                    tileGfxSize = { 8,8 };
                    if ((n & TL) == TL && !(n & (T | L)))
                    {
                        texPos = { 17,34 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize},{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize/2 },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize/2 },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                    }
                    if ((n & TR) == TR && !(n & (T | R)))
                    {
                        texPos = { 8,34 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize},{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize /2 },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize/2 },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                    }
                    if ((n & BL) == BL && !(n & (B | L)))
                    {
                        texPos = { 17,25 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize / 2 }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize / 2 },{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                    }
                    if ((n & BR) == BR && !(n & (B | R)))
                    {
                        texPos = { 8,25 };
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize / 2 }, texPos }); // top left
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize / 2 },{ texPos.x + tileGfxSize.x, texPos.y } }); // top right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texPos + tileGfxSize }); // bottom right
                        verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize/2, pos.y + y * TileSize + TileSize },{ texPos.x, texPos.y + tileGfxSize.y } }); // bottom left

                    }
                }
            }
    }
    m_drawList[ent.getIndex()].verts = verts;
    m_drawList[ent.getIndex()].bounds = verts.getBounds();
}

void TerrainRenderer::onEntityRemoved(xy::Entity ent)
{
    auto verts = m_drawList.find(ent.getIndex());
    if (verts != m_drawList.end())
        m_drawList.erase(verts);
}

void TerrainRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (auto& chunk : m_drawList)
    {
        states.texture = m_sheetTexture;
        rt.draw(chunk.second.verts, states);
    }
}

xy::Entity TerrainRenderer::addChunk(sf::Vector2i index)
{
    xy::Logger::log("Adding chunk at " + std::to_string(index.x) + "," + std::to_string(index.y));
    auto newChunk = getScene()->createEntity();
    newChunk.addComponent<TerrainChunk>().generate(m_noise, index);
    newChunk.addComponent<xy::Transform>().setPosition(sf::Vector2f( index.x * TileSize * ChunkSize, index.y * TileSize * ChunkSize  ));
    
    return newChunk;
}