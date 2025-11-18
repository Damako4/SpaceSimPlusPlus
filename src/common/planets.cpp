#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <soil2/SOIL2.h>

#include <common.hpp>
#include <structs.h>

#include <string>
#include <planets.h>
#include <tools/vboindexer.h>
#include <controls.h>
#include <tools/loadShader.hpp>

/*
TODO
OPTIMIZE TO BE ABLE TO COPY PLANET DATA
SLOW TO GENERATE VERTICES FOR EVERY PLANET MADE
*/

static int planetID = 1;

Planet::Planet(const std::string &title, TextureInfo info, glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, float planetRadius, float planetMass) {
    id = planetID++;
    if (planetID == 1) planetID = 1;
    position_modelSpace = glm::dvec3(position);  // Real-world position
    this->velocity = glm::dvec3(velocity); // Real-world velocity
    this->acceleration = glm::dvec3(acceleration);
    mass = planetMass;
    radius = planetRadius;
    name = title;

    textureInfo = info;
    textureInfo.useTexture = info.useTexture;
    
    if (textureInfo.useTexture && !textureInfo.texturePath.empty()) {
        std::string fullPath = "../src/textures/" + textureInfo.texturePath;
        textureInfo.texture = SOIL_load_OGL_texture(
            fullPath.c_str(), 
            SOIL_LOAD_AUTO, 
            SOIL_CREATE_NEW_ID, 
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );
        if (textureInfo.texture == 0) {
            std::cerr << "Failed to load texture: " << fullPath << std::endl;
            textureInfo.useTexture = false;
        }
    }
    
    if (!textureInfo.useTexture) {
        color = info.color;
    } else {
        color = glm::vec3(1.0f, 1.0f, 1.0f); // Default white
    }

    glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
    generateSphereVertices(1.0f, 36, 18, indices, vertices, normals, uv);

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);

}

void Planet::generateSphereVertices(float radius, int sectorCount, int stackCount,
                            std::vector<unsigned int>& indices,
                            std::vector<glm::vec3>& vertices,
                            std::vector<glm::vec3>& normals,
                            std::vector<glm::vec2>& uv) {
    std::vector<glm::vec3> pre_vertices;
    std::vector<glm::vec3> pre_normals;
    std::vector<glm::vec2> pre_uv;

    float x, y, z, xy;
    float lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = (glm::pi<float>() / 2) - (i * stackStep);
        xy = radius * std::cos(stackAngle);
        z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * std::cos(sectorAngle);
            y = xy * std::sin(sectorAngle);

            pre_vertices.push_back(glm::vec3(x, y, z));
            pre_normals.push_back(glm::vec3(x * lengthInv, y * lengthInv, z * lengthInv));

            // Fixed UV coordinates
            s = (float)j / (float)sectorCount;  // Simple linear mapping from 0 to 1
            t = (float)i / (float)stackCount;   // Simple linear mapping from 0 to 1
            pre_uv.push_back(glm::vec2(s, t));
        }
    }

    // Generate indices (same as before)
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexVBO(pre_vertices, pre_uv, pre_normals, indices, vertices, uv, normals);
}

void Planet::render() {
    
	glUseProgram(state.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint diffuseColorLoc = glGetUniformLocation(state.programID, "diffuseColor");
    glUniform3fv(diffuseColorLoc, 1, &color[0]);

    glUniform1i(glGetUniformLocation(state.programID, "useTexture"), textureInfo.useTexture);

    if (textureInfo.useTexture && textureInfo.texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInfo.texture);
        glUniform1i(glGetUniformLocation(state.programID, "textureSampler"), 0);
    }

    glBindVertexArray(VertexArrayID);

	glDrawElements(
		GL_TRIANGLES,
		indices.size(),
		GL_UNSIGNED_INT,
		static_cast<void *>(nullptr)
	);

    glBindVertexArray(0);
}


glm::vec3 Planet::getPlanetScreenCoords() const {
    glm::vec4 clipSpacePos = state.ProjectionMatrix * state.ViewMatrix * glm::vec4(getScaledPosition(), 1.0f);
    
    if (clipSpacePos.w <= 0.0f) {
        return glm::vec3(-1000, -1000, -1); // Off screen
    }
    
    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

    float x = (ndcSpacePos.x + 1.0f) * 0.5f * static_cast<float>(WIDTH * 2);
    float y = (ndcSpacePos.y + 1.0f) * 0.5f * static_cast<float>(HEIGHT * 2);

    return glm::vec3(x, y, ndcSpacePos.z);
}

glm::vec3 Planet::getScaledPosition() const {
    return glm::vec3(position_modelSpace / SCALE_FACTOR);
}

float Planet::getMass() const {
    return mass;
}

float Planet::getRadius() const {
    return radius;
}