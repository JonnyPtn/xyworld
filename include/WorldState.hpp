#pragma once

#include <xyginext/core/State.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/resources/Resource.hpp>

#include "States.hpp"

#include "FastNoise.h"

class WorldState final : public xy::State
{
public:
    WorldState(xy::StateStack&, xy::State::Context);

    xy::StateID stateID() const override { return States::WorldPlayState; }

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

private:

    xy::Scene m_scene;
    xy::TextureResource m_textures;
    xy::FontResource m_fonts;

    xy::Entity m_player;
};