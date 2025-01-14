#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include "C:/Users/danne/Desktop/ACG/Project/milestone 2/GameEngine/GameEngine/Dependencies/glm/glm.hpp"
// change this folder when you download from repo
// VC is finicky and doesn't want to open this file in path prop
// for some reason...

// 1) Enum for identifying derived collidables
enum class CollidableType {
    BOAT,
    BOX,
    // add other types here if needed...
};

class Collidable {
public:
    virtual ~Collidable() = default;

    // Returns the position of the object
    virtual glm::vec3 getPosition() const = 0;

    // Returns the size (radius for sphere, extents for AABB)
    virtual float getCollisionRadius() const = 0;

    // Action to perform on collision
    virtual void onCollision(const Collidable& other) = 0;

    // 2) Return the collidable's type (no need for dynamic_cast)
    virtual CollidableType getType() const = 0;
};

#endif // COLLIDABLE_H
