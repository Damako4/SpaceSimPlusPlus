#include <rendering/shadow.h>
#include <cstddef>
#include <cmath>
#include <common.hpp>

std::shared_ptr<Shader> ShadowMap::shadowShader = nullptr;

ShadowMap::ShadowMap(int width, int height)
{
    glGenFramebuffers(1, &fbo);  

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void ShadowMap::renderShadowMap()
{
    // Adjust these values to fit your scene better
    float near_plane = 0.1f, far_plane = 20.0f;
    
    // Smaller ortho projection = tighter shadows that match object size
    // This should cover roughly a 4x4 unit area around the origin
    float orthoSize = 10.0f; // Made smaller - adjust if needed (1.0 = very tight, 3.0 = looser)
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
    
    // Light looks at center of scene (where your objects are)
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Center of your scene
    glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // If light is directly above, adjust up vector to avoid gimbal lock
    glm::vec3 lightDir = glm::normalize(lightTarget - state.lights[0].position);
    if (std::abs(glm::dot(lightDir, lightUp)) > 0.99f) {
        lightUp = glm::vec3(1.0f, 0.0f, 0.0f); // Use different up vector
    }
    
    glm::mat4 lightView = glm::lookAt(state.lights[0].position, lightTarget, lightUp);
    lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shadowShader->id);
    
    // Disable culling for shadow map to avoid issues with single-sided geometry like the floor
    glDisable(GL_CULL_FACE);
    
    GLint lightSpaceMatrixLoc = shadowShader->getUniform("lightSpaceMatrix");
    if (lightSpaceMatrixLoc != -1) {
        glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    }
    
    // TODO : Combine Planet and Object class to the same std::vector and iterate through that
    for (Object& obj : state.objects) {
        obj.renderDepth(shadowShader);
    }
    /*
    for (Planet& planet : state.planets) {
        planet.renderDepth(shadowShader);
    }
    */
    
    glEnable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthMap);
}