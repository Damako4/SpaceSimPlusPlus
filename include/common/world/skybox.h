#ifndef SKYBOX_H
#define SKYBOX_H

#include <world/object.h>

class SkyBox : public Object {
    public:
    SkyBox(std::shared_ptr<Shader> shaderProgram);
    void render();
};

#endif