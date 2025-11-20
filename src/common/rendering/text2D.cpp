#include <cstring>
#include <vector>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec2.hpp>
#include <soil2/SOIL2.h>

#include <common.hpp>

#include <rendering/text2D.h>
#include <tools/loadShader.hpp>
#include <iostream>

Text2D::Text2D(std::string texturePath, std::shared_ptr<Shader> shaderProgram) : Object(shaderProgram)
{
    textureInfo.useTexture = true;
    std::string fullPath = "../src/textures/" + texturePath;
    textureInfo.texture = SOIL_load_OGL_texture(
        fullPath.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    if (textureInfo.texture == 0)
    {
        std::cerr << "Failed to load texture: " << fullPath << std::endl;
        textureInfo.useTexture = false;
    }
    // Generate VAO and buffers
    glGenVertexArrays(1, &i_vao);
    glGenBuffers(1, &i_vertexBuffer);
    glGenBuffers(1, &i_uvBuffer);
}

void Text2D::render(const std::string &text, int x, int y, int size)
{
    unsigned int length = text.length();

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;

    for (unsigned int i = 0; i < length; i++)
    {
        // Calculate character position
        int charX = x + i * size;

        // Skip characters that are completely off-screen
        if (charX + size < 0 || charX > WIDTH * 2 || y + size < 0 || y > HEIGHT * 2)
        {
            continue;
        }

        auto vertex_up_left = glm::vec2(charX, y + size);
        auto vertex_up_right = glm::vec2(charX + size, y + size);
        auto vertex_down_right = glm::vec2(charX + size, y);
        auto vertex_down_left = glm::vec2(charX, y);

        vertices.push_back(vertex_up_left);
        vertices.push_back(vertex_down_left);
        vertices.push_back(vertex_up_right);

        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);

        const char character = text[i];
        float uv_x = static_cast<float>(character % 16) / 16.0f;
        float uv_y = static_cast<float>(character / 16) / 16.0f;

        glm::vec2 uv_up_left = glm::vec2(uv_x, 1.0f - uv_y);
        glm::vec2 uv_up_right = glm::vec2(uv_x + 1.0f / 16.0f, 1.0f - uv_y);
        glm::vec2 uv_down_right = glm::vec2(uv_x + 1.0f / 16.0f, 1.0f - (uv_y + 1.0f / 16.0f));
        glm::vec2 uv_down_left = glm::vec2(uv_x, 1.0f - (uv_y + 1.0f / 16.0f));

        UVs.push_back(uv_up_left);
        UVs.push_back(uv_down_left);
        UVs.push_back(uv_up_right);

        UVs.push_back(uv_down_right);
        UVs.push_back(uv_up_right);
        UVs.push_back(uv_down_left);
    }

    // Don't render if no visible characters
    if (vertices.empty())
    {
        return;
    }

    // Bind VAO before setting up vertex attributes
    glBindVertexArray(i_vao);

    glBindBuffer(GL_ARRAY_BUFFER, i_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, i_uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUseProgram(shader->id);

    const GLint widthID = shader->uniforms["width"];
    const GLint heightID = shader->uniforms["height"];

    glUniform1i(widthID, WIDTH);
    glUniform1i(heightID, HEIGHT);

    const GLint textureSamplerID = shader->uniforms["textureSampler"];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureInfo.texture);
    glUniform1i(textureSamplerID, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // VAO is already bound with vertex attributes set up
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisable(GL_BLEND);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);
}