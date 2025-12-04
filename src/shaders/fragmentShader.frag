#version 330 core

in VS_OUT {
    vec2 UV;
    vec3 position_worldSpace;
    vec3 eyeDirection_cameraSpace;
    vec3 normal_cameraSpace;
    vec4 position_lightSpace;
} fs_in;

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

uniform sampler2D depthMap;

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

#define PCF_KERNEL_RADIUS 2

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightPos_cameraSpace = (V * vec4(light.position, 1.0)).xyz;
    vec3 lightDir = normalize(lightPos_cameraSpace - fragPos);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // attenuation
    float distance = length(lightPos_cameraSpace - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    // combine results - multiply by baseColor here for proper material response
    vec3 ambient  = light.ambient * material.ambient * baseColor;
    vec3 diffuse  = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec * material.specular; // specular typically doesn't use baseColor
    
    // Apply attenuation and intensity
    diffuse  *= attenuation * light.intensity;
    specular *= attenuation * light.intensity;

    // Shadow calculation
    vec3 projCoords = fs_in.position_lightSpace.xyz / fs_in.position_lightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range
    
    // Check if fragment is outside light frustum (no shadow)
    if(projCoords.z > 1.0)
    {
        return ambient + diffuse + specular;
    }
    
    // Calculate bias
    vec3 worldNormal = normalize(mat3(transpose(inverse(V))) * normal);
    vec3 worldLightDir = normalize(light.position - fs_in.position_worldSpace);
    float cosTheta = dot(worldNormal, worldLightDir);
    
    float bias = max(0.01 * (1.0 - cosTheta), 0.005);

    // PCF Soft Shadows 
    float shadow = 0.0;
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -PCF_KERNEL_RADIUS; x <= PCF_KERNEL_RADIUS; ++x)
    {
        for(int y = -PCF_KERNEL_RADIUS; y <= PCF_KERNEL_RADIUS; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= float((PCF_KERNEL_RADIUS * 2 + 1) * (PCF_KERNEL_RADIUS * 2 + 1));   

    return ambient + (1.0 - shadow) * (diffuse + specular);
} 

void main() {
    vec3 baseColor = material.diffuse;
    if (useTexture) {
        baseColor = texture(textureSampler, fs_in.UV).rgb;
    }

    vec3 norm = normalize(fs_in.normal_cameraSpace);
    vec3 viewDir = normalize(fs_in.eyeDirection_cameraSpace);

    vec3 result = vec3(0.0);
    
    vec3 position_cameraSpace = (V * vec4(fs_in.position_worldSpace, 1.0)).xyz;
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, position_cameraSpace, viewDir, baseColor);    
    
    color = vec4(result, alpha);  // Use 'color', not 'FragColor'
}