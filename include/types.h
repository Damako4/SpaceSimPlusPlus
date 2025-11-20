#ifndef TYPES_H
#define TYPES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

struct TextureInfo {
    bool useTexture;
    GLint textureSamplerID = -1;
    std::string texturePath;
    GLuint texture;
    glm::vec3 color;
};

enum class ViewMode {
    FREE,
    ORBIT
};

enum class ControlMode {
    MOVE,
    SCALE,
    VIEW,
    NONE
};

struct CameraState {
    glm::vec3 position = glm::vec3(4, 4, 4);
    float horizontalAngle = 90.0f;
    float verticalAngle = 0.0f;
    float orbitRadius = 20.0f;
};

#endif // TYPES_H