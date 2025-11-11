#include "common/controls.h"

#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <structs.h>
#include <unordered_map>
#include <thread>
#include <vector>
#include <iostream>
#include <common/planets.h>

using namespace glm;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

static GLuint pickingFBO = 0;

glm::mat4 getViewMatrix()
{
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{
	return ProjectionMatrix;
}

// Camera state
struct CameraState {
    glm::vec3 position = glm::vec3(4, 4, 4);
    float horizontalAngle = -90.0f;
    float verticalAngle = 0.0f;
} freeCamState;

// Initial Field of View
float initialFoV = FOV;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

// Track previous view mode to handle transitions
static ViewMode previousViewMode = ViewMode::FREE;

bool isKeyPressedOnce(int key, GLFWwindow *window)
{
	static std::unordered_map<int, bool> keyHeld;

	bool pressed = glfwGetKey(window, key) == GLFW_PRESS;

	if (pressed && !keyHeld[key])
	{
		keyHeld[key] = true;
		return true; // first frame the key is pressed
	}
	else if (!pressed)
	{
		keyHeld[key] = false;
	}
	return false;
}

void orbitCamera(GLFWwindow* window, Planet& planet, float orbitRadius)
{
    static double lastTime = glfwGetTime();
    static float orbitAngle = 0.0f;
    
    const double currentTime = glfwGetTime();
    const float deltaTime = static_cast<float>(currentTime - lastTime);
    
    float orbitHeight = 3.0f; // Height above the planet
    const float ORBIT_SPEED = 3.0f; // Reduced speed constant
    
    // Only change angle when keys are pressed
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        orbitAngle += ORBIT_SPEED * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        orbitAngle -= ORBIT_SPEED * deltaTime;
    }
    
    // Normalize angle to prevent floating point issues
    if (orbitAngle > 2.0f * M_PI)
    {
        orbitAngle -= 2.0f * M_PI;
    }
    else if (orbitAngle < -2.0f * M_PI)
    {
        orbitAngle += 2.0f * M_PI;
    }

    // Calculate camera position relative to planet's position
    glm::vec3 cameraPosition = planet.getScaledPosition() + glm::vec3(
        orbitRadius * cos(orbitAngle),
        orbitHeight,
        orbitRadius * sin(orbitAngle)
    );

    // Calculate direction vector pointing from camera to planet center
    glm::vec3 direction = glm::normalize(planet.getScaledPosition() - cameraPosition);

    // Calculate right and up vectors for the camera
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
    glm::vec3 up = glm::cross(direction, right);

    // Projection matrix (you might want to set this elsewhere)
    float FoV = initialFoV; // Ensure initialFoV is defined elsewhere
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

    // View matrix using lookAt
    ViewMatrix = glm::lookAt(
        cameraPosition,              // Camera position orbiting planet
        planet.getScaledPosition(),  // Look at planet center
        up                          // Up vector
    );

    lastTime = currentTime;
}


void handleStateChange(GLFWwindow *window, ControlState &state)
{
	// Implement state change handling logic here
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		state.controlMode = ControlMode::VIEW;
		state.viewMode = ViewMode::ORBIT;
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		state.controlMode = ControlMode::VIEW;
		state.viewMode = ViewMode::FREE;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		state.controlMode = ControlMode::MOVE;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		state.controlMode = ControlMode::SCALE;
	}
	if (isKeyPressedOnce(GLFW_KEY_V, window))
	{
		state.gridVisible = !state.gridVisible;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		state.controlMode = ControlMode::NONE;
	}
	for (int i = 1; i < 10; i++)
	{
		if (isKeyPressedOnce(GLFW_KEY_0 + i, window))
		{
			// Select planet with ID i
			state.selectedPlanet = nullptr;
			for (Planet &planet : state.planets)
			{
				if (planet.id == i)
				{
					state.selectedPlanet = &planet;
					break;
				}
			}
		}
	}

    if (state.controlMode != ControlMode::NONE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if (state.viewMode == ViewMode::FREE)
			{
				centerCamera(window, state.planets);
			} else if (state.viewMode == ViewMode::ORBIT && state.selectedPlanet != nullptr)
			{
				orbitCamera(window, *state.selectedPlanet, 6.0f);
			}
		} else {
			//std::cout << "No control mode active" << std::endl;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
}


void computeMatricesFromInputs(GLFWwindow *window)
{
    static double lastTime = glfwGetTime();
    const double currentTime = glfwGetTime();
    const auto deltaTime = static_cast<float>(currentTime - lastTime);

    // Get mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Reset mouse position for next frame
    glfwSetCursorPos(window, static_cast<float>(WIDTH) / 2, static_cast<float>(HEIGHT) / 2);

    // Compute new orientation
    freeCamState.horizontalAngle += mouseSpeed * static_cast<float>(static_cast<float>(WIDTH) / 2 - xpos);
    freeCamState.verticalAngle += mouseSpeed * static_cast<float>(static_cast<float>(HEIGHT) / 2 - ypos);

    // Clamp vertical angle
    freeCamState.verticalAngle = glm::clamp(freeCamState.verticalAngle, -1.0f, 1.0f);

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    const glm::vec3 direction(
        cos(freeCamState.verticalAngle) * sin(freeCamState.horizontalAngle),
        sin(freeCamState.verticalAngle),
        cos(freeCamState.verticalAngle) * cos(freeCamState.horizontalAngle));

    // Right vector
    const auto right = glm::vec3(
        sin(freeCamState.horizontalAngle - 3.14f / 2.0f),
        0,
        cos(freeCamState.horizontalAngle - 3.14f / 2.0f));

    // Up vector
    const glm::vec3 up = glm::cross(right, direction);

    // Handle movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        freeCamState.position += direction * deltaTime * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        freeCamState.position -= direction * deltaTime * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        freeCamState.position += right * deltaTime * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        freeCamState.position -= right * deltaTime * speed;
    }

    const float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Update matrices
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    ViewMatrix = glm::lookAt(
        freeCamState.position,
        freeCamState.position + direction,
        up
    );

    lastTime = currentTime;
}

glm::vec3 centerOfMass(std::vector<Planet> planets) {
	glm::vec3 average;
	for (Planet planet: planets) {
		average += planet.getScaledPosition();
	}
	average /= static_cast<float>(planets.size());
	return average;
}

void centerCamera(GLFWwindow *window, std::vector<Planet>& planets) {
	glm::vec3 com = centerOfMass(planets);
	freeCamState.position = com + glm::vec3(0.0f, 5.0f, 10.0f);
	ViewMatrix = glm::lookAt(
        freeCamState.position,              // Camera position orbiting planet
        com,  // Look at planet center
        glm::vec3(0, 1, 0)                          // Up vector
    );
}