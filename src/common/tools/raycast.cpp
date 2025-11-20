#include <iostream>
#include <thread>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tools/raycast.h>
#include <tools/loadShader.hpp>
#include <planets.h>
#include <rendering/shader.h>
#include <memory.h>

void raycast(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        state.raycastHandler->cast();
    }
}

Raycast::Raycast(std::shared_ptr<Shader> shaderProgram) : Object(shaderProgram) {
    glGenVertexArrays(1, &i_vao);
}

void Raycast::cast() {
// Clear with white (ID 0 = no selection)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader->id);

        glBindVertexArray(i_vao);
        glEnableVertexAttribArray(0);

        for (Planet &planet : state.planets)
        {
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), planet.getScaledPosition());
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(planet.getRadius()));
            glm::mat4 MVP = Shader::ProjectionMatrix * Shader::ViewMatrix * ModelMatrix;

            glUniformMatrix4fv(shader->uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);

            // Encode planet ID into RGB components
            float r = float(planet.id & 0xFF) / 255.0f;               // Lower 8 bits
            float g = float((planet.id >> 8) & 0xFF) / 255.0f;       // Middle 8 bits
            float b = float((planet.id >> 16) & 0xFF) / 255.0f;      // Upper 8 bits
            
            glUniform4f(shader->uniforms["pickColor"], r, g, b, 1.0f);

            planet.renderRay();
        }

        glDisableVertexAttribArray(0);
        glBindVertexArray(0);

        glFlush();
        glFinish();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        unsigned char data[4];
        double mouseX, mouseY;
        GLFWwindow* window = glfwGetCurrentContext();
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

        glUseProgram(0);
        
        // Find matching planet
        bool found = false;
        for (const Planet& planet : state.planets) {
            if (planet.id == pickedID) {
                std::cout << "Picked planet ID: " << pickedID << std::endl;
                state.selectedPlanet = const_cast<Planet*>(&planet);
                found = true;
                break;
            }
        }
        
        if (!found) {
            state.selectedPlanet = nullptr;
        }
}

