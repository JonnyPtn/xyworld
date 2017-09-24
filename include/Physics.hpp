#pragma once

#include <xyginext/ecs/System.hpp>

class Physics : public xy::System
{
public:

    Physics(xy::MessageBus&);
    void process(float) override;

private:

};