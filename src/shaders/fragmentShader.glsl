#version 330 core

in vec2 UV;
in vec3 normal_cameraSpace;
in vec3 lightDirection_cameraSpace;
in vec3 eyeDirection_cameraSpace;

out vec4 color;

uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 diffuseColor;
uniform float ambientLightIntensity;
uniform vec3 specularColor;
uniform int specularLobeWidth;
uniform float alpha;

uniform sampler2D textureSampler;
uniform bool useTexture;

void main() {
    vec3 baseColor = diffuseColor;
    if (useTexture) {
        
        baseColor = texture(textureSampler, UV).rgb;
    }

    vec3 ambientColor = vec3(ambientLightIntensity) * baseColor;

    // Normal of fragment in camera space
    vec3 n = normalize(normal_cameraSpace);
    // Direction of light from fragment to light
    vec3 l = normalize(lightDirection_cameraSpace);

    // Eye vector towards camera
    vec3 e = normalize(eyeDirection_cameraSpace);
    // Direction triangle reflects light
    vec3 r = reflect(-l, n);

    float cosA = clamp(dot(e,r), 0, 1);

    float cosT = clamp(dot(n,l), 0, 1);

    float distance = length(lightDirection_cameraSpace);
    color.a = alpha;
    vec3 ambient = vec3(ambientLightIntensity) * baseColor;
    vec3 diffuse = baseColor * lightColor * lightIntensity * cosT / (distance * distance);
    vec3 specular = specularColor * lightColor * lightIntensity * pow(cosA, specularLobeWidth) / (distance * distance);

    color = vec4(ambient + diffuse + specular, alpha);
}