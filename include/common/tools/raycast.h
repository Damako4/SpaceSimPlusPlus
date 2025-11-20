#ifndef RAYCAST_H
#define RAYCAST_H

#include <memory>
#include <world/object.h>
#include <common/common.hpp>

class Raycast : public Object
{
    public:
        Raycast(std::shared_ptr<Shader> shaderProgram);

        void cast();
};

void raycast(GLFWwindow* window, int button, int action, int mods);

#endif //RAYCAST_H
