//
// Created by damir on 10/30/24.
//
#include <common/common.hpp>
#include "common/world.h"

#include <vector>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>

#include "loadShader.hpp"
#include <structs.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/controls.h>

// Grid resources
GLuint gridVAO = 0;
GLuint gridVBO = 0;
GLuint programID = 0;
GLint mvpID = -1;
GLint lineColorID = -1;
std::vector<float> gridVertices;
float gridSpacing = 1.0f;
float gridSize = 10.0f;

// Axis resources	
GLuint axisVAO;
GLuint axisVBO;
std::vector<float> axisVertices = {
    // X axis (red)
    0.0f, 0.0f, 0.0f,    // origin
    1.0f, 0.0f, 0.0f,    // x unit vector
    
    // Y axis (green)
    0.0f, 0.0f, 0.0f,    // origin
    0.0f, 1.0f, 0.0f,    // y unit vector
    
    // Z axis (blue)
    0.0f, 0.0f, 0.0f,    // origin
    0.0f, 0.0f, 1.0f     // z unit vector
};

extern glm::mat4 getViewMatrix();
extern glm::mat4 getProjectionMatrix();

void initAxis() {
    // Load shaders if not already loaded
    if (programID == 0) {
        programID = LoadShaders("../src/shaders/lines/lineVertexShader.glsl", "../src/shaders/lines/lineFragmentShader.glsl");
        mvpID = glGetUniformLocation(programID, "MVP");
        lineColorID = glGetUniformLocation(programID, "lineColor");
    }

    // Create and set up VAO/VBO for axes
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    
    // Set up vertex attributes (3 floats per vertex)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawAxis(glm::vec3 position, float scale) {
    // Calculate MVP matrix for this planet
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(scale * 1.5f)); // Scale axis relative to planet size
    glm::mat4 mvp = getProjectionMatrix() * getViewMatrix() * model;
    
    // Use the line shader program
    glUseProgram(programID);
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
    
    // Bind the axis VAO
    glBindVertexArray(axisVAO);
    
    // Draw X axis (red)
    glUniform4f(lineColorID, 1.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    // Draw Y axis (green)
    glUniform4f(lineColorID, 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 2, 2);
    
    // Draw Z axis (blue)
    glUniform4f(lineColorID, 0.0f, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);
    
    // Just unbind VAO, let caller handle program switching
    glBindVertexArray(0);
    
}

void initWorldGrid() {
    if (programID == 0) {
        programID = LoadShaders("../src/shaders/lines/lineVertexShader.glsl", "../src/shaders/lines/lineFragmentShader.glsl");
        mvpID = glGetUniformLocation(programID, "MVP");
        lineColorID = glGetUniformLocation(programID, "lineColor");
    }

    for (float i = -gridSize; i <= gridSize; i += gridSpacing) {
        // Lines parallel to X axis
        gridVertices.push_back(-gridSize); gridVertices.push_back(0.0f); gridVertices.push_back(i);
        gridVertices.push_back(gridSize);  gridVertices.push_back(0.0f); gridVertices.push_back(i);

        // Lines parallel to Z axis
        gridVertices.push_back(i); gridVertices.push_back(0.0f); gridVertices.push_back(-gridSize);
        gridVertices.push_back(i); gridVertices.push_back(0.0f); gridVertices.push_back(gridSize);
    }

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes (3 floats per vertex)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawWorldGrid(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix, glm::vec3 center) {
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * glm::translate(glm::mat4(1.0f), center);

    glUseProgram(programID);
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniform4f(lineColorID, 0.5f, 0.5f, 0.5f, 1.0f); // Grey color for grid lines

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 3));
    glBindVertexArray(0);
}