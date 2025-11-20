#include <world/grid.h>
#include <common.hpp>
#include <vector>
#include <iostream>
#include <tools/loadShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <planets.h>
#include <memory>

Grid::Grid(int size, float spacing, std::shared_ptr<Shader> shaderProgram) : Object(shaderProgram), gridSize(size), gridSpacing(spacing)
{
    float y = 0.0f;
    int pointsPerRow = (2 * gridSize / gridSpacing) + 1;
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

    glGenVertexArrays(1, &i_vao);

    glGenBuffers(1, &i_vertexBuffer);
    glBindVertexArray(i_vao);
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(glm::vec3), gridVertices.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &i_elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_elementBuffer);
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
    
    glBindVertexArray(i_vao);
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gridVertices.size() * sizeof(glm::vec3), gridVertices.data());
    glBindVertexArray(0);
}

void Grid::render()
{                
    glUseProgram(shader->id);
    glUniformMatrix4fv(shader->uniforms["MVP"], 1, GL_FALSE, &getMVP()[0][0]);

    glUniform4f(shader->uniforms["lineColor"], 1, 1, 1, 1);

    glBindVertexArray(i_vao);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, static_cast<void*>(nullptr));
    glBindVertexArray(0);

    glUseProgram(0);
}