#include <unordered_map>
#include <thread>
#include <vector>
#include <iostream>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <controls.h>
#include <planets.h>
#include <rendering/shader.h>
#include <rendering/render.h>

using namespace glm;

static GLuint pickingFBO = 0;

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

void orbitCamera(GLFWwindow *window, glm::vec3 planetPosition, float orbitRadius)
{
    static double lastTime = glfwGetTime();
    static float orbitAngle = 0.0f;

    const double currentTime = glfwGetTime();
    const float deltaTime = static_cast<float>(currentTime - lastTime);

    float orbitHeight = 3.0f;       // Height above the planet
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

    if (orbitAngle > 2.0f * M_PI)
    {
        orbitAngle -= 2.0f * M_PI;
    }
    else if (orbitAngle < -2.0f * M_PI)
    {
        orbitAngle += 2.0f * M_PI;
    }

    glm::vec3 cameraPosition = planetPosition + glm::vec3(
                                                                orbitRadius * cos(orbitAngle),
                                                                orbitHeight,
                                                                orbitRadius * sin(orbitAngle));

    state.freeCamState.position = cameraPosition;

    glm::vec3 direction = glm::normalize(planetPosition - cameraPosition);

    state.freeCamState.horizontalAngle = atan2(direction.x, direction.z);
    state.freeCamState.verticalAngle = asin(direction.y);

    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::cross(right, direction);

    float FoV = initialFoV;

    auto [fbWidth, fbHeight] = getViewportSize();
    Shader::ProjectionMatrix = glm::perspective(glm::radians(initialFoV), float(fbWidth) / float(fbHeight), 0.1f, 100.0f);
    Shader::ViewMatrix = glm::lookAt(state.freeCamState.position, state.freeCamState.position + direction, up);

    lastTime = currentTime;
}

void handleStateChange(GLFWwindow *window)
{
    
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        state.controlMode = ControlMode::VIEW;
        state.viewMode = ViewMode::ORBIT;
    }
    if (isKeyPressedOnce(GLFW_KEY_N, window))
    {
        state.normalsVisible = !state.normalsVisible;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        state.controlMode = ControlMode::VIEW;
        state.viewMode = ViewMode::FREE;

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glfwSetCursorPos(window, width / 2, height / 2);

    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        state.controlMode = ControlMode::MOVE;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
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
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 10.0f;
    }
    else
    {
        speed = 3.0f;
    }
    for (int i = 1; i < 10; i++)
    {
        if (isKeyPressedOnce(GLFW_KEY_0 + i, window))
        {
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
        if (state.viewMode == ViewMode::FREE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            computeMatricesFromInputs(window);
        }
        else if (state.viewMode == ViewMode::ORBIT)
        {
            if (state.selectedPlanet != nullptr) {
                orbitCamera(window, state.selectedPlanet->getScaledPosition(), 6.0f);
            } else {
                orbitCamera(window, centerOfMass(state.planets), state.freeCamState.orbitRadius);
            }
        }
    }
    else
    {
        orbitCamera(window, vec3(0.0f), state.freeCamState.orbitRadius);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void computeMatricesFromInputs(GLFWwindow *window)
{
    static double lastTime = glfwGetTime();

    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glfwSetCursorPos(window, width / 2, height / 2);

    float mouseSpeed = 0.005f;

    state.freeCamState.horizontalAngle += mouseSpeed * float(width / 2 - xpos);
    state.freeCamState.verticalAngle += mouseSpeed * float(height / 2 - ypos);

    const float vlimit = glm::half_pi<float>() - 0.01f;
    state.freeCamState.verticalAngle = glm::clamp(state.freeCamState.verticalAngle, -vlimit, vlimit);

    glm::vec3 direction(
        cos(state.freeCamState.verticalAngle) * sin(state.freeCamState.horizontalAngle),
        sin(state.freeCamState.verticalAngle),
        cos(state.freeCamState.verticalAngle) * cos(state.freeCamState.horizontalAngle));

    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::cross(right, direction);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        state.freeCamState.position += direction * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        state.freeCamState.position -= direction * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        state.freeCamState.position += right * deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        state.freeCamState.position -= right * deltaTime * speed;

    // Use framebuffer size for proper aspect ratio on high-DPI displays
    auto [fbWidth, fbHeight] = getViewportSize();
    Shader::ProjectionMatrix = glm::perspective(glm::radians(initialFoV), float(fbWidth) / float(fbHeight), 0.1f, 100.0f);
    Shader::ViewMatrix = glm::lookAt(state.freeCamState.position, state.freeCamState.position + direction, up);

    lastTime = currentTime;
}

glm::vec3 centerOfMass(std::vector<Planet> planets)
{
    glm::vec3 average;
    for (Planet planet : planets)
    {
        average += planet.getScaledPosition();
    }
    average /= static_cast<float>(planets.size());
    return average;
}

void centerCamera(GLFWwindow *window, std::vector<Planet> &planets)
{
    glm::vec3 com = centerOfMass(planets);
    state.freeCamState.position = com + glm::vec3(0.0f, 5.0f, 10.0f);
    auto [fbWidth, fbHeight] = getViewportSize();
    Shader::ProjectionMatrix = glm::perspective(glm::radians(initialFoV), float(fbWidth) / float(fbHeight), 0.1f, 100.0f);

    Shader::ViewMatrix = glm::lookAt(
        state.freeCamState.position, // Camera position orbiting planet
        com,                         // Look at planet center
        glm::vec3(0, 1, 0)           // Up vector
    );
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    state.freeCamState.orbitRadius = glm::clamp(static_cast<float>(state.freeCamState.orbitRadius + yoffset * SCROLL_SENSITIVITY), 2.0f, 50.0f);
}
