#include <world/skybox.h>
#include <soil2/SOIL2.h>
#include <tools/loadShader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

SkyBox::SkyBox() {;
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
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
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

    glGenVertexArrays(1, &VertexArrayID);
    glGenBuffers(1, &vertexBuffer);
    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    state.cubemapProgramID = LoadShaders("../src/shaders/skybox/skyboxVertexShader.glsl", "../src/shaders/skybox/skyboxFragmentShader.glsl");

    Ploc = glGetUniformLocation(state.cubemapProgramID, "projection");
    Vloc = glGetUniformLocation(state.cubemapProgramID, "view");

    skyboxLoc = glGetUniformLocation(state.cubemapProgramID, "skybox");
}

void SkyBox::render() {
    glDepthMask(GL_FALSE);
    
    glUseProgram(state.cubemapProgramID);

    glBindVertexArray(VertexArrayID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureInfo.texture);

    glUniformMatrix4fv(Ploc, 1, GL_FALSE, &state.ProjectionMatrix[0][0]);
    glm::mat4 view = glm::mat4(glm::mat3(state.ViewMatrix));
    glUniformMatrix4fv(Vloc, 1, GL_FALSE, &view[0][0]);

    glUniform1i(skyboxLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);

    glUseProgram(0);
}