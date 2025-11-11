#version 330 core

layout(location = 0) in vec3 vertexPosition_modelSpace;
layout(location = 1) in vec3 vertexNormal_modelSpace;
layout(location = 2) in vec2 vertexUV;

out vec2 UV;
out vec3 position_worldSpace;
out vec3 eyeDirection_cameraSpace;
out vec3 lightDirection_cameraSpace;
out vec3 normal_cameraSpace;

uniform vec3 lightPosition_worldSpace;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

void main() {
    gl_Position = MVP * vec4(vertexPosition_modelSpace, 1);

    position_worldSpace = (M * vec4(vertexPosition_modelSpace,1)).xyz;

    vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelSpace,1)).xyz;
    eyeDirection_cameraSpace = vec3(0,0,0) - vertexPosition_cameraspace;

    vec3 LightPosition_cameraspace = ( V * vec4(lightPosition_worldSpace,1)).xyz;
    lightDirection_cameraSpace = LightPosition_cameraspace + eyeDirection_cameraSpace;

    normal_cameraSpace = ( V * M * vec4(vertexNormal_modelSpace,0)).xyz;

    UV = vertexUV;
}