#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <cstdlib>
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/geometric.hpp"
#include <string>
#include "glm/glm.hpp"
#include <vector>
#include "print.h"
#include "skybox.h"
#include "color.h"
#include "intersect.h"
#include "object.h"
#include "sphere.h"
#include "cube.h"
#include "light.h"
#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "SDL_image.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 3;
const float BIAS = 0.0001f;

SDL_Renderer* renderer;
std::vector<Object*> objects;
Camera camera(glm::vec3(0.0, 0.0, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 2.0f);
Skybox skybox("../texturas/fondo.png");
Light light(
        glm::vec3(-20.0, -30, 30),   // Posición de la luz
        1.5f,                        // Intensidad de la luz
        Color(255, 255, 255, 255)    // Color de la luz
);



void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i;
        if (dynamic_cast<Cube*>(object) != nullptr) {
            i = dynamic_cast<Cube*>(object)->rayIntersect(rayOrigin, rayDirection);
        } else if (dynamic_cast<Sphere*>(object) != nullptr) {
            i = dynamic_cast<Sphere*>(object)->rayIntersect(rayOrigin, rayDirection);
        }
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    if (!intersect.isIntersecting || recursion == MAX_RECURSION) {
        return skybox.getColor(rayDirection);  // Sky color
    }


    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(rayOrigin - intersect.point);
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);

    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specReflection = glm::dot(viewDir, reflectDir);

    Material mat = hitObject->material;

    float specLightIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);

    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (mat.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        reflectedColor = castRay(origin, reflectDir, recursion + 1);
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (mat.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDir = glm::refract(rayDirection, intersect.normal, mat.refractionIndex);
        refractedColor = castRay(origin, refractDir, recursion + 1);
    }

    Color diffuseLight = mat.diffuse * light.intensity * diffuseLightIntensity * mat.albedo * shadowIntensity;
    Color specularLight = light.color * light.intensity * specLightIntensity * mat.specularAlbedo * shadowIntensity;
    Color color = (diffuseLight + specularLight) * (1.0f - mat.reflectivity - mat.transparency) + reflectedColor * mat.reflectivity + refractedColor * mat.transparency;
    return color;
}

void setUp() {
    // Nuevos materiales para roca
    Material metal1 = {Color(60, 65, 83), 0.8, 0.2, 10.0f, 0.0f, 0.0f};
    Material metal2 = {Color(51, 56, 68), 0.8, 0.2, 10.0f, 0.0f, 0.0f};

    Material madera1 = {Color(114, 67, 40), 0.6, 0.4, 20.0f, 0.0f, 0.0f};
    Material madera2 = {Color(105, 52, 29), 0.6, 0.4, 20.0f, 0.0f, 0.0f};

    Material luz1 = {Color(255, 180, 0, 225), 0.0, 0.0, 5.0f, 0.8f, 1.0f};
    Material luz2 = {Color(255, 150, 0, 225), 0.0, 0.0, 5.0f, 0.8f, 1.0f};
    Material luz3 = {Color(255, 120, 0, 225), 0.0, 0.0, 5.0f, 0.8f, 1.0f};
    Material luz4 = {Color(255, 90, 0, 225), 0.0, 0.0, 5.0f, 0.8f, 1.0f};
    Material luz5 = {Color(255, 60, 0, 225), 0.0, 0.0, 5.0f, 0.8f, 1.0f};

    //suelo1
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(2.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(3.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 0.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));


    //columna1
    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    //columna2
    objects.push_back(new Cube(glm::vec3(6.0f, 2.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(6.0f, 3.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 4.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 5.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 6.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    //columna3
    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(0.0f, 3.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 4.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 5.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(0.0f, 6.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    //columna4
    objects.push_back(new Cube(glm::vec3(6.0f, 2.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(6.0f, 3.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 4.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 5.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(6.0f, 6.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));



    //cara1
    objects.push_back(new Cube(glm::vec3(2.0f, 2.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(3.0f, 2.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(5.0f, 2.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(2.0f, 3.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(3.0f, 3.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(4.0f, 3.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(5.0f, 3.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(2.0f, 4.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(3.0f, 4.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(4.0f, 4.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(5.0f, 4.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(2.0f, 5.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(3.0f, 5.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(4.0f, 5.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(5.0f, 5.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(2.0f, 6.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));
    objects.push_back(new Cube(glm::vec3(3.0f, 6.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(4.0f, 6.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(5.0f, 6.0f, -0.01f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));


    //cara2
    objects.push_back(new Cube(glm::vec3(2.0f, 2.0f, 6.1f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(3.0f, 2.0f, 6.1f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(4.0f, 2.0f, 6.1f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(5.0f, 2.0f, 6.1f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(2.0f, 3.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(3.0f, 3.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(4.0f, 3.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(5.0f, 3.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(2.0f, 4.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(3.0f, 4.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(4.0f, 4.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(5.0f, 4.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(2.0f, 5.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(3.0f, 5.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(4.0f, 5.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(5.0f, 5.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(2.0f, 6.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz5));
    objects.push_back(new Cube(glm::vec3(3.0f, 6.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(4.0f, 6.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(5.0f, 6.0f, 6.1), glm::vec3(1.0f, 1.0f, 1.0f), luz5));



    //cara3
    objects.push_back(new Cube(glm::vec3(6.1f, 2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(6.1f, 2.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1f, 2.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1f, 2.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(6.1, 3.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1, 3.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(6.1, 3.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(6.1, 3.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(6.1, 4.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1, 4.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(6.1, 4.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(6.1, 4.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(6.1, 5.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(6.1, 5.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1, 5.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(6.1, 5.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(6.1, 6.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));
    objects.push_back(new Cube(glm::vec3(6.1, 6.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(6.1, 6.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(6.1, 6.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));


    //cara4
    objects.push_back(new Cube(glm::vec3(-0.1f, 2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(-0.1f, 2.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1f, 2.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1f, 2.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(-0.1, 3.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1, 3.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(-0.1, 3.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(-0.1, 3.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(-0.1, 4.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1, 4.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz1));
    objects.push_back(new Cube(glm::vec3(-0.1, 4.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz2));
    objects.push_back(new Cube(glm::vec3(-0.1, 4.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));

    objects.push_back(new Cube(glm::vec3(-0.1, 5.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(-0.1, 5.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1, 5.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz3));
    objects.push_back(new Cube(glm::vec3(-0.1, 5.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));

    objects.push_back(new Cube(glm::vec3(-0.1, 6.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));
    objects.push_back(new Cube(glm::vec3(-0.1, 6.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(-0.1, 6.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz4));
    objects.push_back(new Cube(glm::vec3(-0.1, 6.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), luz5));


    //techo
    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(2.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(3.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(0.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(2.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(3.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(6.0f, 7.0f, 6.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));


    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(3.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(4.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));

    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));

    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));

    objects.push_back(new Cube(glm::vec3(2.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));
    objects.push_back(new Cube(glm::vec3(3.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(4.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera1));
    objects.push_back(new Cube(glm::vec3(5.0f, 8.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), madera2));



    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(3.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

    objects.push_back(new Cube(glm::vec3(2.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));
    objects.push_back(new Cube(glm::vec3(3.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(4.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal1));
    objects.push_back(new Cube(glm::vec3(5.0f, 9.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f), metal2));

}

void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {

            float random_value = static_cast<float>(std::rand())/static_cast<float>(RAND_MAX);
            if (random_value < 0.0 ) {
                continue;
            }



            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);


            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);

            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                    cameraDir + cameraX * screenX + cameraY * screenY
            );

            Color pixelColor = castRay(camera.position, rayDirection);
            /* Color pixelColor = castRay(glm::vec3(0,0,20), glm::normalize(glm::vec3(screenX, screenY, -1.0f))); */

            point(glm::vec2(x, y), pixelColor);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }



    // Create a window
    SDL_Window *window = SDL_CreateWindow("Raytracer - FPS: 0",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;

    setUp();


    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        print("up");
                        camera.rotate(0.0f, 1.0f);
                        break;
                    case SDLK_DOWN:
                        print("down");
                        camera.rotate(0.0f, -1.0f);
                        break;
                    case SDLK_LEFT:
                        print("left");
                        camera.rotate(-1.0f, 0.0f);
                        break;
                    case SDLK_RIGHT:
                        print("right");
                        camera.rotate(1.0f, 0.0f);
                        break;
                    case SDLK_w:
                        camera.move(1.0f);
                        break;
                    case SDLK_s:
                        camera.move(-1.0f);
                        break;


                }
            }


        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render();

        // Present the renderer
        SDL_RenderPresent(renderer);

        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "Raytracer - FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}