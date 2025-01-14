#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;

// Sunlight properties
uniform vec3 sunColor;  
uniform vec3 sunPos;

// Torchlight properties
uniform vec3 torchColor;
uniform vec3 torchPos;
uniform float torchAmbientStrength;
uniform float torchSpecularStrength;

// Camera position for specular calculations
uniform vec3 viewPos;

void main()
{
    vec3 normal = normalize(norm);

    // Sunlight calculations
    vec3 lightDirSun = normalize(sunPos - fragPos);
    float diffSun = max(dot(normal, lightDirSun), 0.0f);
    vec3 diffuseSun = diffSun * sunColor;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDirSun = reflect(-lightDirSun, normal);
    float specSun = pow(max(dot(viewDir, reflectDirSun), 0.0), 32);
    vec3 specularSun = 0.5 * specSun * sunColor;

    vec3 ambientSun = 0.2 * sunColor;

    // Torchlight calculations
    vec3 lightDirTorch = normalize(torchPos - fragPos);
    float diffTorch = max(dot(normal, lightDirTorch), 0.0f);
    vec3 diffuseTorch = diffTorch * torchColor;

    vec3 reflectDirTorch = reflect(-lightDirTorch, normal);
    float specTorch = pow(max(dot(viewDir, reflectDirTorch), 0.0f), 16);
    vec3 specularTorch = torchSpecularStrength * specTorch * torchColor;

    // Distance-based attenuation for the torchlight
    float distanceTorch = length(torchPos - fragPos);
    float attenuationTorch = 1.0 / (1.0 + 0.045 * distanceTorch + 0.0075 * (distanceTorch * distanceTorch));

    vec3 ambientTorch = torchAmbientStrength * torchColor; // Ambient light is constant
    vec3 resultTorch = (ambientTorch + diffuseTorch + specularTorch) * attenuationTorch;

    // Combine lighting
    vec3 totalLight = ambientSun + diffuseSun + specularSun + resultTorch;

    // Texture color
    vec4 texColor = texture(texture1, textureCoord);

    if (texColor.a < 0.1)
        discard;

    fragColor = vec4(texColor.rgb * totalLight, texColor.a);
}
