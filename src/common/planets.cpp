#include <string>
#include <common/planets.h>
#include <structs.h>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <common/vboindexer.h>
#include <common/common.hpp>
#include <iostream>

/*
TODO
OPTIMIZE TO BE ABLE TO COPY PLANET DATA
SLOW TO GENERATE VERTICES FOR EVERY PLANET MADE
*/

int planetID = 1;

Planet::Planet(GLuint progID, const std::string &name, TextureInfo info, glm::vec3 position, glm::vec3 velocity, glm::vec3 acceleration, float planetRadius, float planetMass) {
    id = planetID++;
    if (planetID == 1) planetID = 1;
    programID = progID;
    // Store REAL-WORLD values directly (no scaling in constructor)
    position_modelSpace = glm::dvec3(position);  // Real-world position in meters
    this->velocity = glm::dvec3(velocity);       // Real-world velocity in m/s
    this->acceleration = glm::dvec3(acceleration);
    mass = planetMass;
    radius = planetRadius;

    textureInfo = info;
    textureInfo.useTexture = info.useTexture;
    
    // Set color from texture info
    if (!info.useTexture) {
        color = info.color;
    } else {
        color = glm::vec3(1.0f, 1.0f, 1.0f); // Default white for textured objects
    }
    

    glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
    // Fill the member 'indices' and the vertex arrays
    generateSphereVertices(1.0f, 36, 18, indices, vertices, normals, uv);

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    // Upload the actual indices stored in the object's member 'indices'
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);

    // 'indices' is already populated by generateSphereVertices via the member reference
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

    // generate (stackCount + 1) rows and (sectorCount + 1) columns so the
    // first and last sector vertices coincide — this closes the seam.
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
            
            s = (float) j / sectorCount;
            t = (float) i / stackCount;
            pre_uv.push_back(glm::vec2(s, t));
        }
    }

    // generate CCW index list of sphere triangles
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

void Planet::draw() {
	glUseProgram(programID);

	// Enable alpha transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint diffuseColorLoc = glGetUniformLocation(programID, "diffuseColor");
    glUniform3fv(diffuseColorLoc, 1, &color[0]);

    glUniform1i(glGetUniformLocation(programID, "useTexture"), textureInfo.useTexture);

    glBindVertexArray(VertexArrayID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

	glDrawElements(
		GL_TRIANGLES,
		indices.size(),
		GL_UNSIGNED_INT,
		static_cast<void *>(nullptr)
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}


glm::vec3 Planet::getPlanetScreenCoords(ControlState& state) {
    glm::vec4 clipSpacePos = state.ProjectionMatrix * state.ViewMatrix * glm::vec4(getScaledPosition(), 1.0f);
    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

    // Convert NDC to window coordinates
    float x = (ndcSpacePos.x + 1.0f) * 0.5f * static_cast<float>(WIDTH);
    float y = (ndcSpacePos.y + 1.0f) * 0.5f * static_cast<float>(HEIGHT);

    return glm::vec3(x, y, ndcSpacePos.z);
}

glm::vec3 Planet::getScaledPosition() const {
    // Convert from real-world meters to OpenGL units for rendering
    return glm::vec3(position_modelSpace / SCALE_FACTOR);
}

float Planet::getMass() {
    return mass;
}

float Planet::getRadius() {
    return radius;
}