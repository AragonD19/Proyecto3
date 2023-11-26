#pragma once

#include "glm/glm.hpp"
#include "color.h"

struct Light {
    glm::vec3 position;
    float intensity;
    Color color;

    Light(const glm::vec3& pos, float inten, const Color& col)
            : position(pos), intensity(inten), color(col) {}
};