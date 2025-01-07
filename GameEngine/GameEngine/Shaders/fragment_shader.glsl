#version 400

in vec3 fragPos;
in vec3 norm;
in vec2 textureCoord;

out vec4 fragColor;

// General texture
uniform sampler2D texture1;

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
    // Normalize the normal
    vec3 normal = normalize(norm);

    // ------------------------------------------------------
    // 1) Sun lighting (directional or distant point with no attenuation)
    // ------------------------------------------------------
    float specularStrength = 0.5;  // Adjust as desired
    vec3 ambientSun = ambientStrength * sunColor;

    vec3 lightDirSun = normalize(sunPos - fragPos);
    float diffSun = max(dot(normal, lightDirSun), 0.0);
    vec3 diffuseSun = diffSun * sunColor;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDirSun = reflect(-lightDirSun, normal);
    float specSun = pow(max(dot(viewDir, reflectDirSun), 0.0), 32);
    vec3 specularSun = specularStrength * specSun * sunColor;

    vec3 sunResult = ambientSun + diffuseSun + specularSun;

    // ------------------------------------------------------
    // 2) Torch lighting (point light with attenuation)
    // ------------------------------------------------------
    float torchAmbientStrength = 0.4; // Or adjust to taste
    vec3 ambientTorch = torchAmbientStrength * torchColor;

    vec3 lightDirTorch = normalize(torchPos - fragPos);
    float diffTorch = max(dot(normal, lightDirTorch), 0.0);
    vec3 diffuseTorch = diffTorch * torchColor;

    vec3 reflectDirTorch = reflect(-lightDirTorch, normal);
    float specTorch = pow(max(dot(viewDir, reflectDirTorch), 0.0), 32);
    vec3 specularTorch = specularStrength * specTorch * torchColor;

    // Basic attenuation
    float distanceTorch = length(torchPos - fragPos);
    float constant  = 1.0;
    float linear    = 0.05;
    float quadratic = 0.007;
    float attenuation = 1.0 / (constant + linear * distanceTorch +
                               quadratic * distanceTorch * distanceTorch);

    vec3 torchResult = (ambientTorch + diffuseTorch + specularTorch) * attenuation;

    // ------------------------------------------------------
    // 3) Combine Sun + Torch
    // ------------------------------------------------------
    vec3 totalLight = sunResult + torchResult;

    // ------------------------------------------------------
    // 4) Sample the object's texture
    // ------------------------------------------------------
    vec4 texColor = texture(texture1, textureCoord);

    // ------------------------------------------------------
    // 5) Final fragment color
    // ------------------------------------------------------
    vec3 finalColor = totalLight * texColor.rgb;
    fragColor = vec4(finalColor, texColor.a);
}
