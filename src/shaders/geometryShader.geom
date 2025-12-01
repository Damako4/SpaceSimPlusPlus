#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in VS_OUT {
    vec2 UV;
    vec3 position_worldSpace;
    vec3 eyeDirection_cameraSpace;
    vec3 normal_cameraSpace;
} gs_in[];

out vec2 UV;
out vec3 position_worldSpace;
out vec3 eyeDirection_cameraSpace;
out vec3 normal_cameraSpace;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        
        // Pass through all attributes
        UV = gs_in[i].UV;
        position_worldSpace = gs_in[i].position_worldSpace;
        eyeDirection_cameraSpace = gs_in[i].eyeDirection_cameraSpace;
        normal_cameraSpace = gs_in[i].normal_cameraSpace;
        
        EmitVertex();
    }
    EndPrimitive();
}