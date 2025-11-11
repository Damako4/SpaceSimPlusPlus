//
// Created by damir on 10/29/24.
//

#ifndef VBOINDEXER_H
#define VBOINDEXER_H

#include <common/common.hpp>
#include <vector>
#include <glm/detail/type_vec.hpp>

void indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned int> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
);

#endif //VBOINDEXER_H
