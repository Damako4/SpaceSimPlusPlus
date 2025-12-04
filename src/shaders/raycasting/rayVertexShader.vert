#version 330 core

layout(location = 0) in vec3 vertexPosition_modelSpace;

// TODO :  replace with uniform block
uniform mat4 MVP;

void main(){
    gl_Position =  MVP * vec4(vertexPosition_modelSpace,1); // Transform vertex positions
}