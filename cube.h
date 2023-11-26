#pragma once

#include "glm/glm.hpp"
#include "object.h"
#include "material.h"
#include "intersect.h"

class Cube : public Object {
public:
    Cube(const glm::vec3& minCorner, const glm::vec3& maxCorner, const Material& mat);

    Intersect rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const override;


    // Método para establecer la textura del cubo
    void setTexture(SDL_Texture* tex) {
        texture = tex;
    }
private:
    glm::vec3 minCorner;
    glm::vec3 maxCorner;
    SDL_Texture* texture;
};
