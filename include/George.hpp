#pragma once

#include <xyginext/ecs/System.hpp>

class George : public xy::System
{
public:
    George(xy::MessageBus& mb) : xy::System(mb, typeid(George)) {}
    void process(float) override;
};