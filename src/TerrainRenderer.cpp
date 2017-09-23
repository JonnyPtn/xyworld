
#include "TerrainRenderer.hpp"
#include "TerrainChunk.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>

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
            xy::Logger::log("Chunk removed");
        }
    }

}

void TerrainRenderer::onEntityAdded(xy::Entity ent)
{
    // Gather the tile data from this chunk and create verts for it
    auto chunk = ent.getComponent<TerrainChunk>();
    auto pos = ent.getComponent<xy::Transform>().getPosition();

    sf::VertexArray verts(sf::PrimitiveType::Quads);

    for (int y(0); y < ChunkSize; y++)
    {
        for (int x(0); x < ChunkSize; x++)
        {
                auto i = y * ChunkSize + x;
                // Vague hacky marching squares?
                uint8_t index(0);
                
                // Top left
                index |= chunk.data[i].height > SeaLevel ? 1 : 0;
                index <<= 1;

                // Top Right
                if ((i + 1) % ChunkSize)
                    index |= chunk.data[i + 1].height > SeaLevel ? 1 : 0;
                else
                {
                    auto chunkId = chunk.getIndex();
                    index |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x + 1, chunkId.y* ChunkSize + y) > SeaLevel ? 1 : 0;
                }
                index <<= 1;

                // Bottom Right
                if ((i + 1) % ChunkSize && i < TileCount - ChunkSize)
                    index |= chunk.data[i + 1 + ChunkSize].height > SeaLevel ? 1 : 0;
                else
                {
                    auto chunkId = chunk.getIndex();
                    index |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x + 1, chunkId.y* ChunkSize + y + 1) > SeaLevel ? 1 : 0;
                }
                index <<= 1;

                // Bottom Left
                if (i + ChunkSize < TileCount)
                    index |= chunk.data[i + ChunkSize].height > SeaLevel ? 1 : 0;
                else
                {
                    auto chunkId = chunk.getIndex();
                    index |= m_noise.GetSimplexFractal(chunkId.x* ChunkSize + x, chunkId.y* ChunkSize + y + 1) > SeaLevel ? 1 : 0;
                }
                
                static const sf::Vector2f tileGfxSize(16.f, 16.f);
                static const std::array<sf::Vector2f,16> texCoords
                { {
                    { 51.f,17.f },
                    { 17.f,17.f },
                    { 0.f,17.f },
                    { 51.f,34.f },
                    { 0.f,34.f },
                    { 0.f,0.f }, // err...
                    { 68.f,17.f },
                    { 68.f,34.f },
                    { 17.f,34.f },
                    { 34.f,17.f },
                    { 0.f,0.f }, // err..
                    { 34.f,34.f },
                    { 51.f,0.f },
                    { 34.f,0.f },
                    { 68.f,0.f },
                    { 85.f,17.f },

                } };
                /*if (chunk.data[i].test(IS_LAND))
                    index = 15;
                else
                    index = 0;*/
                verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize }, texCoords[index] }); // top left
                verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize }, { texCoords[index].x + tileGfxSize.x, texCoords[index].y} }); // top right
                verts.append({ sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize },texCoords[index] + tileGfxSize }); // bottom right
                verts.append({ sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize },{ texCoords[index].x, texCoords[index].y + tileGfxSize.y } }); // bottom left
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
    xy::Logger::log("Adding chunk");
    auto newChunk = getScene()->createEntity();
    newChunk.addComponent<TerrainChunk>().generate(m_noise, index);
    newChunk.addComponent<xy::Transform>().setPosition(sf::Vector2f( index.x * TileSize * ChunkSize, index.y * TileSize * ChunkSize  ));
    
    return newChunk;
}