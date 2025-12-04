#ifndef DEBUG_QUAD_H
#define DEBUG_QUAD_H

#include <glad/glad.h>
#include <rendering/shader.h>
#include <memory>

class DebugQuad {
public:
    DebugQuad(std::shared_ptr<Shader> debugShader);
    ~DebugQuad();
    
    void render(GLuint textureID);

private:
    GLuint quadVAO, quadVBO;
    std::shared_ptr<Shader> shader;
    void setupQuad();
};

#endif // DEBUG_QUAD_H
