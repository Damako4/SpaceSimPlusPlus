//
// Created by damir on 10/30/24.
//

#ifndef WORLD_H
#define WORLD_H
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec.hpp>
#include <structs.h>

void drawAxis(glm::vec3 position, float scale);

void initWorldGrid();
void drawWorldGrid(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix, glm::vec3 center);
void initAxis();

#endif //WORLD_H
