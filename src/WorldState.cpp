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
#include "Physics.hpp"
#include "Velocity.hpp"

#include "Input.hpp"
#include "Commands.hpp"

WorldState::WorldState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus()),
    m_textures()
{    
    ctx.renderWindow.setKeyRepeatEnabled(false);

    // Load the player spritesheet
    auto& sheet = m_textures.get("assets/spritesheets/roguelikeChar_transparent.png");
    auto& font = m_fonts.get("assets/ken_fonts/kenpixel.ttf");

    m_scene.addDirector<InputDirector>();

    m_scene.addSystem<xy::CameraSystem>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<xy::CommandSystem>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<TerrainRenderer>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<xy::SpriteRenderer>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<xy::SpriteAnimator>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<xy::TextRenderer>(ctx.appInstance.getMessageBus());
    m_scene.addSystem<Physics>(ctx.appInstance.getMessageBus());

    // Player entity
    xy::SpriteSheet ss;
    ss.loadFromFile("assets/spritesheets/george.spt", m_textures);
    m_player = m_scene.createEntity();

    // George is 48px tall, but our tiles are 16x16
    m_player.addComponent<xy::Transform>().setScale(1.f / 3.f, 1.f / 3.f);
    m_player.addComponent<xy::Sprite>() = ss.getSprite("george");
    m_player.addComponent<xy::SpriteAnimation>();
    m_player.addComponent<xy::CommandTarget>().ID = PlayerOne;

    // Camera entity
    auto cam = m_scene.createEntity();
    cam.addComponent<xy::Transform>().setPosition(8, 8);
    m_player.getComponent<xy::Transform>().addChild(cam.getComponent<xy::Transform>());
    cam.addComponent<xy::Camera>().zoom(5.f);
    m_scene.setActiveCamera(cam);
}

//public
bool WorldState::handleEvent(const sf::Event& evt)
{
    m_scene.forwardEvent(evt);

    if (evt.type == sf::Event::MouseWheelScrolled)
    {
        auto scroll = evt.mouseWheelScroll.delta;
        auto& cam = m_scene.getActiveCamera().getComponent<xy::Camera>();
        if (scroll > 0)
        {
            cam.zoom(1.1);
        }
        else
        {
            cam.zoom(0.9);
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