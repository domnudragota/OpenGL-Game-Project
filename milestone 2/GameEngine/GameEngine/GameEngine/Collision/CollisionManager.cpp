#include "CollisionManager.h"
#include <iostream>

void CollisionManager::addCollidable(Collidable* obj) {
    collidables.push_back(obj);
}

void CollisionManager::updateCollisions() {
    for (size_t i = 0; i < collidables.size(); ++i) {
        for (size_t j = i + 1; j < collidables.size(); ++j) {
            Collidable* objA = collidables[i];
            Collidable* objB = collidables[j];

            glm::vec3 diff = objA->getPosition() - objB->getPosition();
            float distance = glm::length(diff);
            float combinedRadius = objA->getCollisionRadius() + objB->getCollisionRadius();

            if (distance <= combinedRadius) {
                objA->onCollision(*objB);
                objB->onCollision(*objA);
            }
        }
    }
}

void CollisionManager::clearCollidables() {
    collidables.clear();
}
