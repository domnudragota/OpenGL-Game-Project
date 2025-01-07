#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

// Control whether to use torch-specific textures
uniform bool useTorch;

// --- General Textures ---
uniform sampler2D texture1;

// --- Torch Textures ---
uniform sampler2D diffuseMap;
uniform sampler2D emissionMap;

// --- Sun (Light #1) ---
uniform vec3 sunColor;
uniform vec3 sunPos;

// --- Torch (Light #2) ---
uniform vec3 torchColor;
uniform vec3 torchPos;

// Additional uniforms
uniform vec3 viewPos;
uniform float ambientStrength;

void main()
{
    // ------------------------------
    // 1) Lighting calculations (Sun + Torch)
    vec3 normal = normalize(norm);

    // Sun lighting (no attenuation)
    float specularStrength = 0.7;
    vec3 ambientSun = ambientStrength * sunColor;

    vec3 lightDirSun = normalize(sunPos - fragPos);
    float diffSun = max(dot(normal, lightDirSun), 0.0);
    vec3 diffuseSun = diffSun * sunColor;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDirSun = reflect(-lightDirSun, normal);
    float specSun = pow(max(dot(viewDir, reflectDirSun), 0.0), 64);
    vec3 specularSun = specularStrength * specSun * sunColor;

    vec3 sunResult = ambientSun + diffuseSun + specularSun;

    // Torch lighting (point light with attenuation)
    float torchAmbientStrength = 0.4;
    vec3 ambientTorch = torchAmbientStrength * torchColor;

    vec3 lightDirTorch = normalize(torchPos - fragPos);
    float diffTorch = max(dot(normal, lightDirTorch), 0.0);
    vec3 diffuseTorch = diffTorch * torchColor;

    vec3 reflectDirTorch = reflect(-lightDirTorch, normal);
    float specTorch = pow(max(dot(viewDir, reflectDirTorch), 0.0), 64);
    vec3 specularTorch = specularStrength * specTorch * torchColor;

    float distanceTorch = length(torchPos - fragPos);
    float constant = 1.0;
    float linear = 0.05;
    float quadratic = 0.007;
    float attenuation = 1.0 / (constant + linear * distanceTorch + quadratic * (distanceTorch * distanceTorch));

    vec3 torchResult = (ambientTorch + diffuseTorch + specularTorch) * attenuation;

    // Combine sun and torch lighting
    vec3 totalLight = sunResult + torchResult;

    // ------------------------------
    // 2) Texture sampling
    vec4 sampledColor;

    if (!useTorch)
    {
        // For non-torch objects, use the general texture
        sampledColor = texture(texture1, textureCoord);
    }
    else
    {
        // For torch objects, combine diffuse and emission textures
        vec4 diffuseColor = texture(diffuseMap, textureCoord);
        vec4 emissionColor = texture(emissionMap, textureCoord);

        // Add emission color to the diffuse with a boost factor
        float emissionBoost = 2.0; // Adjust as needed for brightness
        sampledColor = diffuseColor + emissionColor * emissionBoost;
    }

    // Handle transparency: discard fragments with low alpha
    if (sampledColor.a < 0.1)
        discard;

    // ------------------------------
    // 3) Apply lighting to the texture color
    vec3 finalColor = totalLight * sampledColor.rgb;

    fragColor = vec4(finalColor, sampledColor.a);
}
