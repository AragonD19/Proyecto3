#include "camera.h"
#include "glm/gtc/quaternion.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float rotationSpeed)
        : position(position), target(target), up(up), rotationSpeed(rotationSpeed)
{}

void Camera::rotate(float deltaX, float deltaY) {
    glm::quat quatRotY = glm::angleAxis(glm::radians(deltaX * rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quatRotX = glm::angleAxis(glm::radians(deltaY * rotationSpeed), glm::vec3(1.0f, 0.0f, 0.0f));

    // Aplicar rotación vertical alrededor del eje X primero
    position = target + quatRotX * (position - target);

    // Luego, aplicar rotación horizontal alrededor del eje Y
    position = target + quatRotY * (position - target);
}

void Camera::move(float deltaZ) {
    glm::vec3 dir = glm::normalize(target - position);
    position += dir * deltaZ;
}