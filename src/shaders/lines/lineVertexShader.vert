#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

layout(std140) uniform Matrices {
    mat4 V;
    mat4 P;
};

mat4 M;

void main() {
    gl_Position = P * V * M * vec4(vertexPosition_modelspace, 1);
}