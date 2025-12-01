#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <soil2/SOIL2.h>

#include <common.hpp>

#include <string>
#include <planets.h>
#include <tools/vboindexer.h>
#include <controls.h>
#include <tools/loadShader.hpp>
#include <types.h>

int Planet::nextID = 1;

Planet::Planet(const std::string &title, TextureInfo info, glm::vec3 position,
               glm::vec3 velocity, glm::vec3 acceleration,
               float planetRadius, float planetMass, std::shared_ptr<Shader> shaderProgram) : Object(info, shaderProgram), mass(planetMass), radius(planetRadius), id(nextID++)
{
    this->position_modelSpace = glm::dvec3(position);
    this->velocity = glm::dvec3(velocity);
    this->acceleration = glm::dvec3(acceleration);
    this->mass = planetMass;
    this->radius = planetRadius;
    this->name = title;

    textureInfo = info;
    textureInfo.useTexture = info.useTexture;

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
    }

    if (!textureInfo.useTexture)
    {
        textureInfo.color = info.color;
    }
    else
    {
        textureInfo.color = glm::vec3(1.0f, 1.0f, 1.0f); // Default white
    }

    glGenVertexArrays(1, &i_vao);
    glBindVertexArray(i_vao);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
    generateSphereVertices(1.0f, 36, 18, indices, vertices, normals, uv);

    glGenBuffers(1, &i_elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}

void Planet::generateSphereVertices(float radius, int sectorCount, int stackCount,
                                    std::vector<unsigned int> &indices,
                                    std::vector<glm::vec3> &vertices,
                                    std::vector<glm::vec3> &normals,
                                    std::vector<glm::vec2> &uv)
{
    std::vector<glm::vec3> pre_vertices;
    std::vector<glm::vec3> pre_normals;
    std::vector<glm::vec2> pre_uv;

    float x, y, z, xy;
    float lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = (glm::pi<float>() / 2) - (i * stackStep);
        xy = radius * std::cos(stackAngle);
        z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;

            x = xy * std::cos(sectorAngle);
            y = xy * std::sin(sectorAngle);

            pre_vertices.push_back(glm::vec3(x, y, z));
            pre_normals.push_back(glm::vec3(x * lengthInv, y * lengthInv, z * lengthInv));

            // Fixed UV coordinates
            s = (float)j / (float)sectorCount; // Simple linear mapping from 0 to 1
            t = (float)i / (float)stackCount;  // Simple linear mapping from 0 to 1
            pre_uv.push_back(glm::vec2(s, t));
        }
    }

    // Generate indices (same as before)
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexVBO(pre_vertices, pre_uv, pre_normals, indices, vertices, uv, normals);
}

void Planet::render()
{
    // TODO: Use the object render method
    // Object::render();
    glUseProgram(shader->id);

    glm::mat4 ObjectModelMatrix = glm::scale(getModelMatrix(), glm::vec3(radius));
    
    glm::mat4 MVP = Shader::ProjectionMatrix * Shader::ViewMatrix * ObjectModelMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(ObjectModelMatrix)));

    glUniformMatrix4fv(shader->uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(shader->uniforms["M"], 1, GL_FALSE, &ObjectModelMatrix[0][0]); // send per-object model matrix
    glUniformMatrix4fv(shader->uniforms["V"], 1, GL_FALSE, &shader->ViewMatrix[0][0]);
    glUniformMatrix3fv(shader->uniforms["normalMatrix"], 1, GL_FALSE, &normalMatrix[0][0]);

    glUniform3fv(shader->uniforms["material.diffuse"], 1, &textureInfo.color[0]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(shader->uniforms["useTexture"], textureInfo.useTexture);

    if (textureInfo.useTexture && textureInfo.texture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInfo.texture);
        glUniform1i(shader->uniforms["textureSampler"], 0);
    }

    glBindVertexArray(i_vao);

    glDrawElements(
        GL_TRIANGLES,
        indices.size(),
        GL_UNSIGNED_INT,
        static_cast<void *>(nullptr));

    glBindVertexArray(0);
    glUseProgram(0);
}

void Planet::renderRay()
{
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_elementBuffer);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);
}

glm::vec3 Planet::getPlanetScreenCoords() const
{
    glm::vec4 clipSpacePos = Shader::ProjectionMatrix * Shader::ViewMatrix * glm::vec4(getScaledPosition(), 1.0f);

    // Off-screen?
    if (clipSpacePos.w <= 0.0f)
        return glm::vec3(-1000, -1000, -1);

    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;
    float x = (ndcSpacePos.x + 1.0f) * 0.5f * static_cast<float>(WIDTH * 2);
    float y = (ndcSpacePos.y + 1.0f) * 0.5f * static_cast<float>(HEIGHT * 2);

    return glm::vec3(x, y, ndcSpacePos.z);
}