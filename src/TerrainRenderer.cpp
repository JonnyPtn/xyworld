
#include "TerrainRenderer.hpp"
#include "TerrainChunk.hpp"

#include "SFML/Graphics/RenderTarget.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>


TerrainRenderer::TerrainRenderer(xy::MessageBus& mb, xy::Scene& scene) :
    xy::System(mb, typeid(TerrainRenderer)),
    m_scene(scene),
    m_noise()
{
    requireComponent<TerrainChunk>();
    requireComponent<xy::Transform>();

    m_noise.SetNoiseType(FastNoise::Cellular);
}


void TerrainRenderer::process(float dt)
{
    // First remove any chunks outside the visible window
    for (auto& ent : getEntities())
    {
        sf::FloatRect bounds = m_drawList[ent.getIndex()].getBounds();
        if (!bounds.contains(m_scene.getActiveCamera().getComponent<xy::Transform>().getPosition()))
        {
            m_scene.destroyEntity(ent);
        }
    }

    // If there's no chunks, spawn one on the camera
    if (!getEntities().size())
    {
        auto pos = m_scene.getActiveCamera().getComponent<xy::Transform>().getPosition();
        auto newChunk = m_scene.createEntity();
        newChunk.addComponent<TerrainChunk>().pos = sf::Vector2i( pos.x / TileSize, pos.y / TileSize );
        newChunk.getComponent<TerrainChunk>().generate(m_noise);
        newChunk.addComponent<xy::Transform>().setPosition({ pos.x - TileSize * (ChunkSize / 2), pos.y - TileSize * (ChunkSize / 2) });
    }
}

void TerrainRenderer::onEntityAdded(xy::Entity ent)
{
    // Gather the tile data from this chunk and create verts for it
    auto chunk = ent.getComponent<TerrainChunk>();
    auto pos = ent.getComponent<xy::Transform>().getPosition();

    sf::VertexArray verts(sf::PrimitiveType::Quads);

    for (int x(0); x < ChunkSize; x++)
    {
        for (int y(0); y < ChunkSize; y++)
        {
            if (chunk.data[x*ChunkSize + y])
            {
                verts.append(sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize }); // top left
                verts.append(sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize }); // top right
                verts.append(sf::Vector2f{ pos.x + x * TileSize + TileSize, pos.y + y * TileSize + TileSize }); // bottom right
                verts.append(sf::Vector2f{ pos.x + x * TileSize, pos.y + y * TileSize + TileSize }); // bottom left
            }
        }
    }
    m_drawList[ent.getIndex()] = verts;
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
        rt.draw(chunk.second, states);
    }
}