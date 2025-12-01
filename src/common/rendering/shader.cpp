#include <rendering/shader.h>
#include <tools/loadShader.hpp>

GLuint Shader::matricesUBO = 0;
glm::mat4 Shader::ViewMatrix = glm::mat4(1.0f);
glm::mat4 Shader::ProjectionMatrix = glm::mat4(1.0f);

Shader::Shader(const std::string &vs, const std::string &fs, const std::string &gs)
{
    id = LoadShaders(("../src/shaders/" + vs).c_str(), ("../src/shaders/" + fs).c_str(), ("../src/shaders/" + gs).c_str());

    GLint count;
    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);

    for (int i = 0; i < count; i++)
    {
        char name[128];
        GLsizei length;
        glGetActiveUniform(id, i, sizeof(name), &length, nullptr, nullptr, name);
        GLint location = glGetUniformLocation(id, name);
        uniforms[name] = location;
    }
}

Shader::Shader(const std::string &vs, const std::string &fs)
{
    id = LoadShaders(("../src/shaders/" + vs).c_str(), ("../src/shaders/" + fs).c_str(), "");

    GLint count;
    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);

    for (int i = 0; i < count; i++)
    {
        char name[128];
        GLsizei length;
        glGetActiveUniform(id, i, sizeof(name), &length, nullptr, nullptr, name);
        GLint location = glGetUniformLocation(id, name);
        uniforms[name] = location;
    }
}

void Shader::initMatricesUBO() {
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);

    
}

void Shader::updateMatricesUBO() {
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &ViewMatrix[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &ProjectionMatrix[0][0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}