#ifndef OBJECTS_H
#define OBJECTS_H
#include <string>

#include <structs.h>

Object initObject(const std::string& filepath, const std::string& texturePath, GLuint programID, GLuint textureSamplerID, const std::string& name, glm::vec3 &xyz);
void drawObject(const Object& object);

#endif //OBJECTS_H
