#ifndef CONTROLS_H
#define CONTROLS_H

#include <common.hpp>
#include <glm/detail/type_mat.hpp>
#include <planets.h>

void computeMatricesFromInputs(GLFWwindow *window);

void handleStateChange(GLFWwindow* window);

void centerCamera(GLFWwindow *window, std::vector<Planet>& planets);

glm::vec3 centerOfMass(std::vector<Planet> planets);

void orbitCamera(GLFWwindow* window, Planet& planet, float orbitRadius);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#endif //CONTROLS_H
