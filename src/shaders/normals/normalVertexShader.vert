#version 330 core

layout(location = 0) in vec3 vertexPosition_modelSpace;
layout(location = 1) in vec3 vertexNormal_modelSpace;

out VS_OUT {
    vec3 normal;
} vs_out;

layout(std140) uniform Matrices {
    mat4 V;
    mat4 P;
};

uniform mat4 M;

void main() {
    gl_Position = V * M * vec4(vertexPosition_modelSpace, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(V * M)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * vertexNormal_modelSpace, 0.0)));
}