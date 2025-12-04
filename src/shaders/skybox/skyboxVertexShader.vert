#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform Matrices {
    mat4 V;
    mat4 P;
};

void main()
{
    TexCoords = aPos;
    // Remove translation from view matrix for skybox
    mat4 view = mat4(mat3(V));
    vec4 pos = P * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // Set z=w to ensure skybox is at far plane
}  