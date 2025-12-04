#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in VS_OUT {
    vec2 UV;
    vec3 position_worldSpace;
    vec3 eyeDirection_cameraSpace;
    vec3 normal_cameraSpace;
    vec4 position_lightSpace;
} gs_in[];

out VS_OUT {
    vec2 UV;
    vec3 position_worldSpace;
    vec3 eyeDirection_cameraSpace;
    vec3 normal_cameraSpace;
    vec4 position_lightSpace;
} gs_out;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        
        // Pass through all attributes using the output block
        gs_out.UV = gs_in[i].UV;
        gs_out.position_worldSpace = gs_in[i].position_worldSpace;
        gs_out.eyeDirection_cameraSpace = gs_in[i].eyeDirection_cameraSpace;
        gs_out.normal_cameraSpace = gs_in[i].normal_cameraSpace;
        gs_out.position_lightSpace = gs_in[i].position_lightSpace;
        
        EmitVertex();
    }
    EndPrimitive();
}