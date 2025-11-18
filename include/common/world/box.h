#ifndef BOX_H
#define BOX_H

#include <glm/glm.hpp>
#include <structs.h>

class Box : private Object
{
public:
    Box(float size);
    int vertexCount;
    void render();

private:
    float boxSize;
};

#endif // BOX_H