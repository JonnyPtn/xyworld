#include "Input.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <SFML/Window/Event.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>

#include "Commands.hpp"
#include <xyginext/util/Vector.hpp>

void InputDirector::handleEvent(const sf::Event& ev)
{
    // The input struct, to be sent out in a command to interested Entities
    if (ev.type == sf::Event::KeyPressed)
    {
        m_pressedKeys.insert(ev.key.code);
    }
    else if (ev.type == sf::Event::KeyReleased)
    {
        m_pressedKeys.erase(ev.key.code);
    }
    else if (ev.type == sf::Event::JoystickMoved)
    {
        // For joysticks, get the id first
        auto& i = m_playerInput[ev.joystickMove.joystickId];

        switch (ev.joystickMove.axis)
        {
        case sf::Joystick::Axis::X:
            i.xy.x = ev.joystickMove.position/100.f;
            break;

        case sf::Joystick::Axis::Y:
            i.xy.y = ev.joystickMove.position/100.f;
        }
    }
}

void InputDirector::process(float dt)
{
    // Handle input with commands
    int player = PlayerOne;
    for (auto& i : m_playerInput)
    {
        xy::Command cmd;
        cmd.targetFlags |= player;
        player <<= 1;
        cmd.action = [&i](xy::Entity e, float dt)
        {
            auto& t = e.getComponent<xy::Transform>();
           
            // basic crappy dead zone
            const float DZ(0.2f);
            if (std::fabs(xy::Util::Vector::length(i.xy)) < DZ)
                i.xy = { 0,0 };

            xy::Logger::log("movement = " + std::to_string(i.xy.x) + " " + std::to_string(i.xy.y));
            //execute move
            t.move(i.xy);

            // Probably shouldn't be doing this here
            // Update animation based on movement
            auto& a = e.getComponent<xy::SpriteAnimation>();

            static int currentAnim;
            int animId(-1);

            //hacky...
            float dx(i.xy.x), dy(i.xy.y);
            if (std::fabs(i.xy.x) > std::fabs(i.xy.y))
                dy = 0.f;
            else
                dx = 0.f;

            // Down
            if (dy > 0.f)
                animId = 0;
            
            // Left
            if (dx < 0.f)
                animId = 1;

            // Up
            if (dy < 0.f)
                animId = 2;

            // Right
            if (dx > 0.f)
                animId = 3;

            if (animId > -1)
            {
                if (animId != currentAnim)
                {
                    a.stop();
                    currentAnim = animId;
                    a.play(animId);
                }
                a.play(animId);
            }
            else
            {
                a.pause();
            }
            
            // Force it to int position to prevent artifacts
            auto pos = t.getPosition();
            pos.x = std::round(pos.x);
            pos.y = std::round(pos.y);
            t.setPosition(pos);
        };

        getScene().getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
}