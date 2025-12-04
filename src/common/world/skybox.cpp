#include <world/skybox.h>
#include <soil2/SOIL2.h>
#include <tools/loadShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <rendering/shader.h>

SkyBox::SkyBox(std::shared_ptr<Shader> shaderProgram) : Object(shaderProgram) {
    glGenTextures(1, &textureInfo.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureInfo.texture);

    std::vector<std::string> textures_faces = {
        "../src/textures/skybox/right.jpg",
        "../src/textures/skybox/left.jpg",
        "../src/textures/skybox/top.jpg",
        "../src/textures/skybox/bottom.jpg",
        "../src/textures/skybox/front.jpg",
        "../src/textures/skybox/back.jpg"
    };

    int width, height, channels;
    unsigned char *data;  
    for(unsigned int i = 0; i < textures_faces.size(); i++)
    {
        data = SOIL_load_image(textures_faces[i].c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
        if (data) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            SOIL_free_image_data(data);
        } else {
            std::cerr << "Failed to load texture: " << textures_faces[i] << std::endl;
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);  // Generate mipmaps for better quality


    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &i_vao);
    glGenBuffers(1, &i_vertexBuffer);
    glBindVertexArray(i_vao);
    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void SkyBox::render() {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    
    glUseProgram(shader->id);

    glBindVertexArray(i_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureInfo.texture);

    auto skybox = shader->getUniform("skybox");
    if (skybox != -1) {
        glUniform1i(skybox, 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glUseProgram(0);
}