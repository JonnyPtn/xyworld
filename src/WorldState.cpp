/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include "WorldState.hpp"

#include <xyginext/core/App.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/NetInterpolation.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/InterpolationSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/graphics/postprocess/ChromeAb.hpp>

#include <xyginext/network/NetData.hpp>
#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "TerrainChunk.hpp"
#include "TerrainRenderer.hpp"

WorldState::WorldState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus()),
    m_textures()
{
    
    // Load the player spritesheet
    auto& sheet = m_textures.get("assets/spritesheets/roguelikeChar_transparent.png");

    m_scene.addSystem<TerrainRenderer>(ctx.appInstance.getMessageBus(), m_scene);
    m_scene.addSystem<xy::SpriteRenderer>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<xy::CameraSystem>(ctx.appInstance.getMessageBus());

    // Player entity
    m_player = m_scene.createEntity();
    m_player.addComponent<xy::Transform>();
    m_player.addComponent<xy::Sprite>().setTexture(sheet);
    m_player.getComponent<xy::Sprite>().setTextureRect({ 0, 0, 16, 16 });
    m_player.addComponent<xy::Camera>().setZoom(5.f);
    m_scene.setActiveCamera(m_player);
}

//public
bool WorldState::handleEvent(const sf::Event& evt)
{
    m_scene.forwardEvent(evt);

    float speed(16.f);

    if (evt.type == sf::Event::KeyPressed)
    {
        switch (evt.key.code)
        {
        case sf::Keyboard::W:
            m_player.getComponent<xy::Transform>().move(0, -speed);
            break;

        case sf::Keyboard::A:
            m_player.getComponent<xy::Transform>().move(-speed,0);
            break;

        case sf::Keyboard::S:
            m_player.getComponent<xy::Transform>().move(0, speed);
            break;

        case sf::Keyboard::D:
            m_player.getComponent<xy::Transform>().move(speed,0);
            break;
        }
        auto& c = m_player.getComponent<xy::Transform>();
        auto pos = c.getPosition();
        std::string newPos = "x=" + std::to_string(pos.x) + ", y=" + std::to_string(pos.y);
        xy::Logger::log("New position: " + newPos, xy::Logger::Type::Info);
    }
    else if (evt.type == sf::Event::MouseWheelScrolled)
    {
        auto scroll = evt.mouseWheelScroll.delta;
        auto& cam = m_scene.getActiveCamera().getComponent<xy::Camera>();
        if (scroll > 0)
        {
            cam.setZoom(1.1);
        }
        else
        {
            cam.setZoom(0.9);
        }
    }
    return false;
}

void WorldState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool WorldState::update(float dt)
{
    xy::NetEvent evt;
    m_scene.update(dt);
    return false;
}

void WorldState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}