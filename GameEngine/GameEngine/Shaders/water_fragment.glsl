#version 400

in vec3 norm;
in vec3 fragPos;
in vec2 textureCoord;

out vec4 fragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 waterColor;

void main()
{
    // Basic Phong shading
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 normal   = normalize(norm);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 diffuse  = diff * lightColor;

    float specularStrength = 0.2; // or higher
    vec3 viewDir    = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular   = specularStrength * spec * lightColor;

    // Combine
    vec3 result = ambient + diffuse + specular;

    // Multiply by the solid-blue texture
    vec4 waterColor = texture(texture1, texCoord);
    fragColor = vec4(lighting, 1.0) * waterColor;
}
