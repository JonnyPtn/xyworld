#include "Physics.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include "Velocity.hpp"

Physics::Physics(xy::MessageBus& mb) :
    xy::System(mb, typeid(Physics))
{
    requireComponent<Velocity>();
    requireComponent<xy::Transform>();
}

void Physics::process(float dt)
{
    for (auto& ent : getEntities())
    {
        ent.getComponent<xy::Transform>().move(ent.getComponent<Velocity>()*dt);
    }
}