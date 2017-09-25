#include "Input.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <SFML/Window/Event.hpp>
#include <xyginext/ecs/components/Transform.hpp>

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
            i.xy.y -= 1.f;
            break;

        case sf::Keyboard::S:
            i.xy.y += 1.f;
            break;

        case sf::Keyboard::A:
            i.xy.x -= 1.f;
            break;
            
        case sf::Keyboard::D:
            i.xy.x += 1.f;
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
           
            t.move(i.xy);
           
        };

        getScene().getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
}