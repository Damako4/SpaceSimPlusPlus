#ifndef SKYBOX_H
#define SKYBOX_H

#include <structs.h>

class SkyBox : public Object {
    public:
    SkyBox();
    void render();

    private:
    GLuint Vloc;
    GLuint Ploc;

    GLuint skyboxLoc;

};

#endif