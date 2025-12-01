#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

typedef struct {
    glm::vec3 position;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    float intensity;
} Light;

#endif // LIGHT_H