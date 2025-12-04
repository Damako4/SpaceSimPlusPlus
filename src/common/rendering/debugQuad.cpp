#include <rendering/debugQuad.h>

DebugQuad::DebugQuad(std::shared_ptr<Shader> debugShader) : shader(debugShader) {
    setupQuad();
}

void DebugQuad::setupQuad() {
    // Screen-space quad vertices with texture coordinates
    // Position in bottom-right corner of screen
    float quadVertices[] = {
        // positions   // texCoords
        0.5f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.0f, 0.0f,
        1.0f,  0.5f,  1.0f, 0.0f,

        0.5f,  1.0f,  0.0f, 1.0f,
        1.0f,  0.5f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

void DebugQuad::render(GLuint textureID) {
    glUseProgram(shader->id);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    GLint depthMapLoc = shader->getUniform("depthMap");
    if (depthMapLoc != -1) {
        glUniform1i(depthMapLoc, 0);
    }
    
    // Optional: set near/far planes for linearization
    GLint nearLoc = shader->getUniform("near_plane");
    if (nearLoc != -1) {
        glUniform1f(nearLoc, 1.0f);
    }
    
    GLint farLoc = shader->getUniform("far_plane");
    if (farLoc != -1) {
        glUniform1f(farLoc, 7.5f);
    }
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

DebugQuad::~DebugQuad() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}
