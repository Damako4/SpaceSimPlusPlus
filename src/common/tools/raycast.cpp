#include <iostream>
#include <thread>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <structs.h>

#include <tools/raycast.h>
#include <tools/loadShader.hpp>
#include <planets.h>

GLuint pickProgramID = 0;
GLuint pickColorID = -1;
GLuint pickMatrixID = -1;
// Dedicated VAO for the picking pass (Core profile requires a VAO bound)
static GLuint pickVAO = 0;

void initRaycastingShader() {
    pickProgramID = LoadShaders("../src/shaders/raycasting/rayVertexShader.glsl", "../src/shaders/raycasting/rayFragmentShader.glsl");
    pickColorID = glGetUniformLocation(pickProgramID, "pickColor");
    pickMatrixID = glGetUniformLocation(pickProgramID, "MVP");
    if (pickVAO == 0) {
        glGenVertexArrays(1, &pickVAO);
    }
}

void raycast(GLFWwindow *window, int button, int action, int mods)
{
    ControlState *state = static_cast<ControlState *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Clear with white (ID 0 = no selection)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(pickProgramID);

        glBindVertexArray(pickVAO);
        glEnableVertexAttribArray(0);

        for (Planet &planet : state->planets)
        {
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), planet.getScaledPosition());
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(planet.getRadius()));
            glm::mat4 MVP = state->ProjectionMatrix * state->ViewMatrix * ModelMatrix;

            glUniformMatrix4fv(pickMatrixID, 1, GL_FALSE, &MVP[0][0]);

            // Encode planet ID into RGB components
            float r = float(planet.id & 0xFF) / 255.0f;               // Lower 8 bits
            float g = float((planet.id >> 8) & 0xFF) / 255.0f;       // Middle 8 bits
            float b = float((planet.id >> 16) & 0xFF) / 255.0f;      // Upper 8 bits
            
            glUniform4f(pickColorID, r, g, b, 1.0f);

            glBindBuffer(GL_ARRAY_BUFFER, planet.vertexBuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet.elementBuffer);

            glDrawElements(GL_TRIANGLES, planet.indices.size(), GL_UNSIGNED_INT, (void*)0);
        }

        glDisableVertexAttribArray(0);
        glBindVertexArray(0);

        glFlush();
        glFinish();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        unsigned char data[4];
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Get both framebuffer and window sizes
        int fbWidth, fbHeight;
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Scale coordinates for high DPI displays
        double scaledX = mouseX * (double)fbWidth / (double)windowWidth;
        double scaledY = mouseY * (double)fbHeight / (double)windowHeight;

        glReadPixels(
            static_cast<int>(scaledX), 
            static_cast<int>(fbHeight - scaledY), 
            1, 1, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            data);

        // Calculate picked ID and validate against actual planet IDs
        int pickedID = data[0] + (data[1] << 8) + (data[2] << 16);
        
        // Find matching planet
        bool found = false;
        for (const Planet& planet : state->planets) {
            if (planet.id == pickedID) {
                std::cout << "Picked planet ID: " << pickedID << std::endl;
                state->selectedPlanet = const_cast<Planet*>(&planet);
                found = true;
                break;
            }
        }
        
        if (!found) {
            state->selectedPlanet = nullptr;
        }

        // Wait 500 ms thread sleep
    }
}