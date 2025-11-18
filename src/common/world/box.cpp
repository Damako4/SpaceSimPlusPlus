#include <world/box.h>
#include <glm/gtc/matrix_transform.hpp>
#include <tools/objloader.h>
#include <soil2/SOIL2.h>
#include <iostream>

Box::Box(float size) : boxSize(size)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    loadOBJ("../src/models/box.obj", vertices, uvs, normals);

    textureInfo.useTexture = true;
    textureInfo.textureSamplerID = glGetUniformLocation(state.programID, "textureSampler");
    std::string fullPath = "../src/textures/box/Wood_Crate_001_basecolor.jpg";
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

    vertexCount = vertices.size();

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}

void Box::render()
{
    glUseProgram(state.programID);
    
    glUniform1i(glGetUniformLocation(state.programID, "useTexture"), textureInfo.useTexture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureInfo.texture);
    glUniform1i(textureInfo.textureSamplerID, 0);

    glm::mat4 ObjectModelMatrix = glm::mat4(1.0f);
    ObjectModelMatrix = glm::translate(ObjectModelMatrix, glm::vec3(position_modelSpace));
    glm::mat4 ObjectMVP = state.ProjectionMatrix * state.ViewMatrix * ObjectModelMatrix;

    glUniformMatrix4fv(state.matrixID, 1, GL_FALSE, &ObjectMVP[0][0]);
    glUniformMatrix4fv(state.modelMatrixID, 1, GL_FALSE, &ObjectModelMatrix[0][0]);

    glBindVertexArray(VertexArrayID);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glBindVertexArray(0);
    glUseProgram(0);
}