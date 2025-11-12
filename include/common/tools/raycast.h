#ifndef RAYCAST_H
#define RAYCAST_H

#include <common/common.hpp>

void initRaycastingShader();

void raycast(GLFWwindow* window, int button, int action, int mods);

#endif //RAYCAST_H
