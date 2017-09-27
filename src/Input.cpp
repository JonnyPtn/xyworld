#include "Input.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <SFML/Window/Event.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>

#include "Commands.hpp"

void InputDirector::handleEvent(const sf::Event& ev)
{
    // The input struct, to be sent out in a command to interested Entities
    if (ev.type == sf::Event::KeyPressed)
    {
        // For keyboard presses, assume player one
        auto& i = m_playerInput[0];
        switch (ev.key.code)
        {
        case sf::Keyboard::W:
            i.xy.y = -1.f;
            break;

        case sf::Keyboard::S:
            i.xy.y = 1.f;
            break;

        case sf::Keyboard::A:
            i.xy.x = -1.f;
            break;
            
        case sf::Keyboard::D:
            i.xy.x = 1.f;
        }

    }
    else if (ev.type == sf::Event::KeyReleased)
    {
        // For keyboard presses, assume player one
        auto& i = m_playerInput[0];
        switch (ev.key.code)
        {
        case sf::Keyboard::W:
            i.xy.y += 1.f;
            break;

        case sf::Keyboard::S:
            i.xy.y -= 1.f;
            break;

        case sf::Keyboard::A:
            i.xy.x += 1.f;
            break;

        case sf::Keyboard::D:
            i.xy.x -= 1.f;
        }
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
            float dx(0.f), dy(0.f);
            if (std::fabs(i.xy.x) > DZ)
                dx = i.xy.x;
            if (std::fabs(i.xy.y) > DZ)
                dy = i.xy.y;

            // Probably shouldn't be doing this here
            // Update animation based on movement
            auto& a = e.getComponent<xy::SpriteAnimation>();

            static int currentAnim;
            int animId(-1);

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
            }
            else
            {
                a.stop();
            }

            //execute move
            t.move(dx, dy);
        };

        getScene().getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
}