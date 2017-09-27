#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <xyginext/ecs/Director.hpp>

#include <array>

#include <set>

struct InputData
{
    sf::Vector2f xy;
};

class InputDirector : public xy::Director
{
public:
    void handleMessage(const xy::Message& msg) override {};
    void handleEvent(const sf::Event&) override;
    void process(float) override;

private:
    std::array<InputData,4> m_playerInput;

    std::set<sf::Keyboard::Key> m_pressedKeys;
};