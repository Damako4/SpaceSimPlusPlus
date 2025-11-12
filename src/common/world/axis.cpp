#include <glm/gtc/matrix_transform.hpp>

#include <common.hpp>

#include <structs.h>
#include <world/axis.h>
#include <controls.h>
#include <planets.h>
#include <tools/loadShader.hpp>
#include <iostream>

Axis::Axis() {
    if (state.lineProgramID == 0) {
        state.lineProgramID = LoadShaders("../src/shaders/lines/lineVertexShader.glsl", "../src/shaders/lines/lineFragmentShader.glsl");
        
        if (state.lineProgramID == 0) {
            std::cerr << "Failed to load line shaders!" << std::endl;
            return;
        }
    }
    
    // Always get uniform locations (in case they weren't set before)
    mvpID = glGetUniformLocation(state.lineProgramID, "MVP");
    lineColorID = glGetUniformLocation(state.lineProgramID, "lineColor");
    
    if (mvpID == -1) {
        std::cerr << "Failed to get MVP uniform location!" << std::endl;
    }
    if (lineColorID == -1) {
        std::cerr << "Failed to get lineColor uniform location!" << std::endl;
    }

    axisVertices = {
        // X axis
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        // Y axis
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // Z axis
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    
    if (axisVAO == 0 || axisVBO == 0) {
        std::cerr << "Failed to generate VAO/VBO for axis!" << std::endl;
        return;
    }
    
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    std::cout << "Axis initialized: VAO=" << axisVAO << ", VBO=" << axisVBO << ", MVP=" << mvpID << ", Color=" << lineColorID << std::endl;
}

void Axis::renderAxis(Planet& planet) {
    // Save current program so we can restore it after drawing the axis
    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), planet.getScaledPosition());
    model = glm::scale(model, glm::vec3(planet.getRadius() * 2.0f)); // Scale axis relative to planet size
    glm::mat4 mvp = getProjectionMatrix() * getViewMatrix() * model;
        
    glUseProgram(state.lineProgramID);
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);

    glBindVertexArray(axisVAO);
    
    glUniform4f(lineColorID, 1.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    glUniform4f(lineColorID, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 2, 2);
    
    glUniform4f(lineColorID, 0.0f, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);
    
    glBindVertexArray(0);

    // Restore previous program so subsequent glUniform calls use the right program
    glUseProgram(static_cast<GLuint>(prevProgram));
}