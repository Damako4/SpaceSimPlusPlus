#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform Matrices {
    mat4 P;
    mat4 V;
};

void main()
{
    TexCoords = aPos;
    gl_Position = P * V * vec4(aPos, 1.0);
}  