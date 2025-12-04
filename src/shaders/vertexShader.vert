#version 330 core

layout(location = 0) in vec3 vertexPosition_modelSpace;
layout(location = 1) in vec3 vertexNormal_modelSpace;
layout(location = 2) in vec2 vertexUV;

out VS_OUT {
    vec2 UV;
    vec3 position_worldSpace;
    vec3 eyeDirection_cameraSpace;
    vec3 normal_cameraSpace;
    vec4 position_lightSpace;
} vs_out;

layout(std140) uniform Matrices {
    mat4 V;
    mat4 P;
};

uniform mat4 M;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

void main() {
    gl_Position = P * V * M * vec4(vertexPosition_modelSpace, 1);

    vs_out.position_worldSpace = (M * vec4(vertexPosition_modelSpace,1)).xyz;

    vs_out.position_lightSpace = lightSpaceMatrix * vec4(vs_out.position_worldSpace, 1);

    vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelSpace,1)).xyz;
    vs_out.eyeDirection_cameraSpace = vec3(0,0,0) - vertexPosition_cameraspace;

    vs_out.normal_cameraSpace = normalize(mat3(V) * normalMatrix * vertexNormal_modelSpace);
    vs_out.UV = vertexUV;
}