#include "camera.h"

// Constructors and Destructor
Camera::Camera(glm::vec3 cameraPosition)
{
    this->cameraPosition = cameraPosition;
    this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
    this->rotationOx = 0.0f;
    this->rotationOy = -90.0f;
}

Camera::Camera()
{
    this->cameraPosition = glm::vec3(0.0f, 0.0f, 100.0f);
    this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
    this->rotationOx = 0.0f;
    this->rotationOy = -90.0f;
}

Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraViewDirection, glm::vec3 cameraUp)
{
    this->cameraPosition = cameraPosition;
    this->cameraViewDirection = cameraViewDirection;
    this->cameraUp = cameraUp;
    this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
}

Camera::~Camera() {}

// Set Camera Position
void Camera::setPosition(const glm::vec3& position)
{
    cameraPosition = position;
}

// Set View Direction
void Camera::setViewDirection(const glm::vec3& direction)
{
    cameraViewDirection = glm::normalize(direction);
    cameraRight = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
}

// Movement Methods
void Camera::keyboardMoveFront(float cameraSpeed)
{
    cameraPosition += cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveBack(float cameraSpeed)
{
    cameraPosition -= cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveLeft(float cameraSpeed)
{
    cameraPosition -= cameraRight * cameraSpeed;
}

void Camera::keyboardMoveRight(float cameraSpeed)
{
    cameraPosition += cameraRight * cameraSpeed;
}

void Camera::keyboardMoveUp(float cameraSpeed)
{
    cameraPosition += cameraUp * cameraSpeed;
}

void Camera::keyboardMoveDown(float cameraSpeed)
{
    cameraPosition -= cameraUp * cameraSpeed;
}

// Rotation Methods
void Camera::rotateOx(float angle)
{
    rotationOx += angle;
    if (rotationOx > 89.0f)
        rotationOx = 89.0f;
    if (rotationOx < -89.0f)
        rotationOx = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    front.y = sin(glm::radians(rotationOx));
    front.z = sin(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    cameraViewDirection = glm::normalize(front);
    cameraRight = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
}

void Camera::rotateOy(float angle)
{
    rotationOy += angle;

    glm::vec3 front;
    front.x = cos(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    front.y = sin(glm::radians(rotationOx));
    front.z = sin(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
    cameraViewDirection = glm::normalize(front);
    cameraRight = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
}

// Get Rotation Angles
float Camera::getRotationOx() const
{
    return rotationOx;
}

float Camera::getRotationOy() const
{
    return rotationOy;
}

// Getters
glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);
}

glm::vec3 Camera::getCameraPosition()
{
    return cameraPosition;
}

glm::vec3 Camera::getCameraViewDirection()
{
    return cameraViewDirection;
}

glm::vec3 Camera::getCameraUp()
{
    return cameraUp;
}
