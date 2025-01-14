#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include <vector>
#include "Collidable.h"

class CollisionManager {
private:
    std::vector<Collidable*> collidables;

public:
    // Add an object to the collision system
    void addCollidable(Collidable* obj);

    // Update collisions for all objects
    void updateCollisions();

    // Clear all registered collidables
    void clearCollidables();
};

#endif // COLLISION_MANAGER_H
