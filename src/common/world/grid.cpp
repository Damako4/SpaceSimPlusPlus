#include <world/grid.h>
#include <common.hpp>
#include <vector>
#include <iostream>
#include <tools/loadShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <planets.h>

Grid::Grid(int size, float spacing) : gridSize(size), gridSpacing(spacing)
{
    if (state.lineProgramID == 0)
    {
        state.lineProgramID = LoadShaders("../src/shaders/lines/lineVertexShader.glsl", "../src/shaders/lines/lineFragmentShader.glsl");
    }

    mvpID = glGetUniformLocation(state.lineProgramID, "MVP");
    lineColorID = glGetUniformLocation(state.lineProgramID, "lineColor");

    float y = 0.0f;
    int pointsPerRow = (2 * gridSize / gridSpacing) + 1;  // Number of points per row
    for (float z = -gridSize; z <= gridSize; z += gridSpacing) {
        for (float x = -gridSize; x <= gridSize; x += gridSpacing) {
            gridVertices.push_back(glm::vec3(x, y, z));
        }
    }

    for (int row = 0; row < pointsPerRow; row++) {
        for (int col = 0; col < pointsPerRow - 1; col++) {
            int current = row * pointsPerRow + col;
            int next = current + 1;
            
            indices.push_back(current);
            indices.push_back(next);
        }
    }

    for (int col = 0; col < pointsPerRow; col++) {
        for (int row = 0; row < pointsPerRow - 1; row++) {
            int current = row * pointsPerRow + col;
            int below = (row + 1) * pointsPerRow + col;
            
            indices.push_back(current);
            indices.push_back(below);
        }
    }

    glGenVertexArrays(1, &VertexArrayID);

    glGenBuffers(1, &vertexBuffer);
    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(glm::vec3), gridVertices.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Grid::update()
{
    for (auto& vertex : gridVertices) {
        float totalDeformation = 0.0f;

        for (const Planet& planet : state.planets) {
            glm::vec2 vertexPos2D(vertex.x, vertex.z);
            glm::vec2 planetPos2D(planet.getScaledPosition().x, planet.getScaledPosition().z);
            float r = glm::distance(vertexPos2D, planetPos2D);
            
            float minDistance = 0.2f;
            r = std::max(r, minDistance);

            float massScale = planet.getMass() / 1e29f;  // Scale mass
            float wellDepth = 2.0f;                      // Maximum depth of well
            float wellRadius = 8.0f;                     // How far the effect extends
            float smoothness = 2.0f;                     // Controls steepness (higher = smoother)
            
            // Smooth gravity well function: exponential decay with gentle slope
            float normalizedDistance = r / wellRadius;
            float deformation = wellDepth * massScale * exp(-normalizedDistance * smoothness) * (1.0f / (1.0f + normalizedDistance));
            
            totalDeformation += deformation;
        }

        vertex.y = -totalDeformation;
    }
    
    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gridVertices.size() * sizeof(glm::vec3), gridVertices.data());
    glBindVertexArray(0);
}

void Grid::render()
{            
    glm::mat4 MVP = state.ProjectionMatrix * state.ViewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    
    glUseProgram(state.lineProgramID);
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);

    glUniform4f(lineColorID, 1, 1, 1, 1);

    glBindVertexArray(VertexArrayID);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, static_cast<void*>(nullptr));
    glBindVertexArray(0);
}