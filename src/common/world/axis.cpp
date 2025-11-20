#include <glm/gtc/matrix_transform.hpp>

#include <common.hpp>

#include <world/axis.h>
#include <controls.h>
#include <planets.h>
#include <tools/loadShader.hpp>
#include <iostream>
#include <memory>

Axis::Axis(std::shared_ptr<Shader> shaderProgram) : Object(shaderProgram) {

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

    glGenVertexArrays(1, &i_vao);
    glGenBuffers(1, &i_vertexBuffer);
    
    glBindVertexArray(i_vao);
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Axis::render(Planet& planet) {
    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

    glm::mat4 model = planet.getModelMatrix();
    model = glm::scale(model, glm::vec3(planet.getRadius() * 2.0f)); // Scale axis relative to planet size

    glUseProgram(shader->id);

    glUniformMatrix4fv(shader->uniforms["MVP"], 1, GL_FALSE, &planet.getMVP()[0][0]);

    glBindVertexArray(i_vao);

    glUniform4f(shader->uniforms["lineColor"], 1.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 0, 2);

    glUniform4f(shader->uniforms["lineColor"], 0.0f, 1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 2, 2);

    glUniform4f(shader->uniforms["lineColor"], 0.0f, 0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);
    
    glBindVertexArray(0);
    
    // Restore previous program so subsequent glUniform calls use the right program
    glUseProgram(static_cast<GLuint>(prevProgram));
}