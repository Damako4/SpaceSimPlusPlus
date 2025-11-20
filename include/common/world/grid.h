#ifndef GRID_H
#define GRID_H

#include <world/object.h>

class Grid : public Object {
    public:
        Grid(int size, float spacing, std::shared_ptr<Shader> shaderProgram);
        void update();
        void render();

    private:
        std::vector<glm::vec3> gridVertices;
        int gridSize;
        float gridSpacing;
        int vertexCount;
};

#endif //GRID_H