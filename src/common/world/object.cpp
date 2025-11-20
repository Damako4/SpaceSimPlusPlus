#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <tools/objloader.h>
#include <soil2/SOIL2.h>
#include <iostream>
#include <world/object.h>

Object::Object(TextureInfo info, std::shared_ptr<Shader> shaderProgram) 
    : textureInfo(info), shader(shaderProgram) {
}

Object::Object(std::shared_ptr<Shader> shaderProgram) 
    : shader(shaderProgram) {
}


Object::Object(std::string modelP, TextureInfo texInfo, std::shared_ptr<Shader> shaderProgram) : textureInfo(texInfo), shader(shaderProgram)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    loadOBJ("../src/models/" + modelP, vertices, uvs, normals);

    if (textureInfo.useTexture && !textureInfo.texturePath.empty())
    {
        std::string fullPath = "../src/textures/" + textureInfo.texturePath;
        textureInfo.texture = SOIL_load_OGL_texture(
            fullPath.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
        if (textureInfo.texture == 0)
        {
            std::cerr << "Failed to load texture: " << fullPath << std::endl;
            textureInfo.useTexture = false;
        }
    } else if (!textureInfo.useTexture) {
        textureInfo.color = texInfo.color;
    } else {
        textureInfo.color = glm::vec3(1.0f, 1.0f, 1.0f); // Default white
    }

    textureInfo.textureSamplerID = shader->uniforms["textureSampler"];
    vertexCount = static_cast<GLuint>(vertices.size());
    glGenVertexArrays(1, &i_vao);
    glBindVertexArray(i_vao);

    glGenBuffers(1, &i_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &i_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, i_normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &i_uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, i_uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}

void Object::render()
{
    glUseProgram(shader->id);

    glUniform1i(shader->uniforms["useTexture"], textureInfo.useTexture);

    if (textureInfo.useTexture && textureInfo.texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInfo.texture);
        glUniform1i(shader->uniforms["textureSampler"], 0);
    } else {
        glUniform3fv(shader->uniforms["diffuseColor"], 1, &textureInfo.color[0]);
    }

    glUniformMatrix4fv(shader->uniforms["MVP"], 1, GL_FALSE, &getMVP()[0][0]);
    glUniformMatrix4fv(shader->uniforms["M"], 1, GL_FALSE, &getModelMatrix()[0][0]);
    glUniformMatrix4fv(shader->uniforms["V"], 1, GL_FALSE, &shader->ViewMatrix[0][0]);
    glUniformMatrix3fv(shader->uniforms["normalMatrix"], 1, GL_FALSE, &getNormalMatrix()[0][0]);

    glBindVertexArray(i_vao);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glBindVertexArray(0);
    glUseProgram(0);
}