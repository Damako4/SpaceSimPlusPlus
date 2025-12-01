#version 330 core

in vec2 UV;
in vec3 normal_cameraSpace;
in vec3 eyeDirection_cameraSpace;
in vec3 position_worldSpace;

out vec4 color;  // Use this, not FragColor

//uniform float lightIntensity;
//uniform float ambientLightIntensity;
uniform float alpha;

layout(std140) uniform Matrices {
    mat4 V;
    mat4 P;
};

uniform sampler2D textureSampler;
uniform bool useTexture;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float intensity;
};  

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightPos_cameraSpace = (V * vec4(light.position, 1.0)).xyz;
    vec3 lightDir = normalize(lightPos_cameraSpace - fragPos);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // attenuation
    float distance = length(lightPos_cameraSpace - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));    
    
    // combine results
    vec3 ambient  = light.ambient * material.ambient * light.intensity;
    vec3 diffuse  = light.diffuse * diff * material.diffuse * light.intensity;
    vec3 specular = light.specular * spec * material.specular * light.intensity;
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
} 

void main() {
    vec3 baseColor = material.diffuse;
    if (useTexture) {
        baseColor = texture(textureSampler, UV).rgb;
    }

    vec3 norm = normalize(normal_cameraSpace);
    vec3 viewDir = normalize(eyeDirection_cameraSpace);

    vec3 result = vec3(0.0);
    
    vec3 position_cameraSpace = (V * vec4(position_worldSpace, 1.0)).xyz;
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, position_cameraSpace, viewDir);    
    
    if (useTexture) {
        result *= baseColor;
    }
    
    color = vec4(result, alpha);  // Use 'color', not 'FragColor'
}