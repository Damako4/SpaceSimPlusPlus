#version 330 core

out vec4 color;

uniform vec4 pickColor;

void main(){
    color = pickColor; // Assign unique color for object picking
}