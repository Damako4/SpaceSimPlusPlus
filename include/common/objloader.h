//
// Created by damir on 10/28/24.
//

#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <string>
#include <vector>
#include <glm/detail/type_vec.hpp>

bool loadOBJ(
	std::string fileName,
	std::vector<glm::vec3> &out_vertices,
	std::vector<glm::vec2> &out_uvs,
	std::vector<glm::vec3> &out_normals
	);

#endif //OBJLOADER_H
