#ifndef SHADOW_H
#define SHADOW_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <rendering/shader.h>
#include <memory>

class ShadowMap {
    public:
        ShadowMap(int width, int height);
        ~ShadowMap();
        void renderShadowMap();
        GLuint getDepthTexture() const { return depthMap; }
        glm::mat4 getLightSpaceMatrix() const { return lightSpaceMatrix; }

        static std::shared_ptr<Shader> shadowShader;

    private:
        GLuint depthMap;
        GLuint fbo;
        glm::mat4 lightSpaceMatrix;
};

#endif // SHADOW_H