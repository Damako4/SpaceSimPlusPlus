#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;

uniform int width;
uniform int height;

void main() {

    vec2 centeredVertexPosition_screenspace = vertexPosition_screenspace - vec2(width, height);
    centeredVertexPosition_screenspace /= vec2(width, height);
    gl_Position = vec4(centeredVertexPosition_screenspace, 0, 1);

    UV = vertexUV;
}