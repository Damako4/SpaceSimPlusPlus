#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    static glm::mat4 ViewMatrix;
    static glm::mat4 ProjectionMatrix;

    GLuint id;
    std::unordered_map<std::string, GLint> uniforms;

    Shader(const std::string& vs, const std::string& fs, const std::string& gs);
    Shader(const std::string& vs, const std::string& fs);

    void setUniform(const std::string& name, const glm::vec3& v) { 
        glUniform3fv(uniforms[name], 1, &v[0]);
    }
    void setUniform(const std::string& name, const glm::mat4& v) {
        glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, &v[0][0]);
    }
    void setUniform(const std::string& name, const glm::mat3& v) {
        glUniformMatrix3fv(uniforms[name], 1, GL_FALSE, &v[0][0]);
    }
};


#endif