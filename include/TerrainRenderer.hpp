#pragma once

#include <array>
#include <unordered_map>
#include <xyginext/ecs/System.hpp>

#include "FastNoise.h"

class TerrainRenderer : public xy::System, public sf::Drawable
{
public:
    TerrainRenderer(xy::MessageBus&, xy::Scene& scene);
    void process(float) override;

private:

    xy::Scene& m_scene;

    // One vert array per chunk
    std::unordered_map<xy::Entity::ID,sf::VertexArray> m_drawList;

    void onEntityAdded(xy::Entity) override;
    void onEntityRemoved(xy::Entity) override;

    void draw(sf::RenderTarget&, sf::RenderStates) const override;

    FastNoise m_noise;
};
