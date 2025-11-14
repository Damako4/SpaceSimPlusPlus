#ifndef GRID_H
#define GRID_H

#include <structs.h>

class Grid : public Object {
    public:
        Grid(int size, float spacing);

        void update();
        void render();
    private:
        GLuint lineColorID;
        std::vector<glm::vec3> gridVertices;
        int gridSize;
        float gridSpacing;
        int vertexCount;
};

#endif //GRID_H