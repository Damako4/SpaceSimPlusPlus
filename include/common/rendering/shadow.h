#ifndef SHADOW_H
#define SHADOW_H

#include <glad/glad.h>

class ShadowMap {
    public:
        ShadowMap(int width, int height);
        ~ShadowMap();

    private:
        GLuint depthMap;
        GLuint fbo;
};

#endif // SHADOW_H