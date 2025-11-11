//
// Created by damir on 10/26/24.
//

#ifndef CONTROLS_H
#define CONTROLS_H

#include <common/common.hpp>
#include <glm/detail/type_mat.hpp>
#include <structs.h>

void computeMatricesFromInputs(GLFWwindow *window);

void handleStateChange(GLFWwindow* window, ControlState& state);

void centerCamera(GLFWwindow *window, std::vector<Planet>& planets);

glm::vec3 centerOfMass(std::vector<Planet> planets);

void orbitCamera(GLFWwindow* window, Planet& planet, float orbitRadius);
glm::mat4 getViewMatrix();

glm::mat4 getProjectionMatrix();

#endif //CONTROLS_H
