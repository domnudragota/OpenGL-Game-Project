#pragma once

#include <glm.hpp>
#include <gtx\transform.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include "..\Graphics\window.h"

class Camera
{
private:
    glm::vec3 cameraPosition;
    glm::vec3 cameraViewDirection;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;

    float rotationOx;  // Pitch (up-down rotation)
    float rotationOy;  // Yaw (left-right rotation)

public:
    // Constructors
    Camera();
    Camera(glm::vec3 cameraPosition);
    Camera(glm::vec3 cameraPosition, glm::vec3 cameraViewDirection, glm::vec3 cameraUp);
    ~Camera();

    // Getters
    glm::mat4 getViewMatrix();
    glm::vec3 getCameraPosition();
    glm::vec3 getCameraViewDirection();
    glm::vec3 getCameraUp();

    float getRotationOx() const;  // Get pitch angle
    float getRotationOy() const;  // Get yaw angle

    // Setters
    void setPosition(const glm::vec3& position);
    void setRotationOx(float angle);  // Set pitch angle
    void setRotationOy(float angle);  // Set yaw angle
    void setViewDirection(const glm::vec3& direction);

    // Movement
    void keyboardMoveFront(float cameraSpeed);
    void keyboardMoveBack(float cameraSpeed);
    void keyboardMoveLeft(float cameraSpeed);
    void keyboardMoveRight(float cameraSpeed);
    void keyboardMoveUp(float cameraSpeed);
    void keyboardMoveDown(float cameraSpeed);

    // Rotation
    void rotateOx(float angle);  // Rotate around X-axis (pitch)
    void rotateOy(float angle);  // Rotate around Y-axis (yaw)
};
