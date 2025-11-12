#ifndef GRID_H
#define GRID_H

#include <structs.h>

class Grid : public Object {
    public:
        Grid(int size, float spacing);

        void init();
        void update();
        void render();
    private:
        GLuint VBO;
        int gridSize;
        float gridSpacing;
};

#endif //GRID_H