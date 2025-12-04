#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

// Linearize depth for better visualization
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depthValue = texture(depthMap, TexCoords).r;
    // For orthographic projection (like your shadow map), you can just use depthValue directly
    // For perspective projection, uncomment the line below:
    // depthValue = LinearizeDepth(depthValue) / far_plane;
    
    FragColor = vec4(vec3(depthValue), 1.0);
}
