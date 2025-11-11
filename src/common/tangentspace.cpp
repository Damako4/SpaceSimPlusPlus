//
// Created by damir on 10/31/24.
//

#include "common/tangentspace.h"

#include <vector>
#include <glm/glm.hpp>

void computeTangentBasis(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> uvs, std::vector<glm::vec3> &normals,
	std::vector<glm::vec3> &tangents, std::vector<glm::vec3> &bitangents) {
	for (unsigned int i = 0; i < vertices.size(); i++) {
		glm::vec3 v0 = vertices[i];
		glm::vec3 v1 = vertices[i+1];
		glm::vec3 v2 = vertices[i+2];

		glm::vec3 uv0 = uvs[i];
		glm::vec3 uv1 = uvs[i+1];
		glm::vec3 uv2 = uvs[i+2];

		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;
	}
}
