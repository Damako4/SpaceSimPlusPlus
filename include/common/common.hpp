#ifndef COMMON_H
#define COMMON_H

#define WIDTH 1024
#define HEIGHT 768
#define FOV 45.0f
#define NEAR_CLIP 0.1f
#define FAR_CLIP 100.0f

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

#define SCROLL_SENSITIVITY 2.0f

#define PHYSICS_SCALE_FACTOR 2.992e10
#define PHYSICS_TIME_SCALE 2000.0f; // 8640 -> 1 day visible orbital motion

const double gravitationalConstant = 6.674e-11;
const double lightConstant = 3e8;

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <rendering/shader.h>
#include <vector>
#include <tools/raycast.h>
#include <planets.h>
#include <world/axis.h>
#include <types.h>
#include "rendering/light.h"

GLuint loadBMP_custom(const char *filepath);
GLuint loadDDS(const char *filepath);

void setup();
void initWorld(Shader defaultShader);

class Raycast;

struct ControlState {
	ViewMode viewMode;
	ControlMode controlMode;
	bool gridVisible = true;
	bool normalsVisible = false;
	std::vector<Planet>& planets;
	Planet* selectedPlanet;

	Light lights[8];
	std::vector<Object>& objects;

	Axis* axisHandler;
	Raycast* raycastHandler;

	CameraState freeCamState;

	ControlState(std::vector<Planet>& planet_ref, std::vector<Object>& object_ref) : planets(planet_ref), objects(object_ref) {

	}
};

extern ControlState state;

#endif //COMMON_H
