#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;

// --- Sun (Light #1) ---
uniform vec3 sunColor;
uniform vec3 sunPos;

// --- Torch (Light #2) ---
uniform vec3 torchColor;
uniform vec3 torchPos;

uniform vec3 viewPos;

// NEW UNIFORM: Y coordinate above which we consider the "flame"
uniform float flameCutoffY;

void main()
{
    // -----------------------------------
    // 1) Sun lighting (minimal attenuation or none)
    float sunAmbientStrength = 0.3;  
    float specularStrength   = 0.7;  
    vec3 normal = normalize(norm);

    // ambient
    vec3 ambientSun = sunAmbientStrength * sunColor;

    // diffuse
    vec3 lightDirSun = normalize(sunPos - fragPos);
    float diffSun = max(dot(normal, lightDirSun), 0.0);
    vec3 diffuseSun = diffSun * sunColor;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDirSun = reflect(-lightDirSun, normal);
    float specSun = pow(max(dot(viewDir, reflectDirSun), 0.0), 64);
    vec3 specularSun = specularStrength * specSun * sunColor;

    vec3 sunResult = ambientSun + diffuseSun + specularSun;


    // -----------------------------------
    // 2) Torch lighting (point light with attenuation)
    float torchAmbientStrength = 0.4;
    vec3 ambientTorch = torchAmbientStrength * torchColor;

    vec3 lightDirTorch = normalize(torchPos - fragPos);
    float diffTorch    = max(dot(normal, lightDirTorch), 0.0);
    vec3 diffuseTorch  = diffTorch * torchColor;

    vec3 reflectDirTorch = reflect(-lightDirTorch, normal);
    float specTorch = pow(max(dot(viewDir, reflectDirTorch), 0.0), 64);
    vec3 specularTorch = specularStrength * specTorch * torchColor;

    // attenuation
    float distanceTorch = length(torchPos - fragPos);
    float constant      = 1.0;
    float linear        = 0.05;
    float quadratic     = 0.007;
    float attenuation   = 1.0 / (constant + linear * distanceTorch
                                 + quadratic * (distanceTorch * distanceTorch));

    vec3 torchResult = (ambientTorch + diffuseTorch + specularTorch) * attenuation;


    // -----------------------------------
    // 3) Combine the two lights
    vec3 totalLight = sunResult + torchResult;

       // -----------------------------------
    // 4) Distinguish flame vs. stick by fragPos.y
    //    If fragPos.y > flameCutoffY, treat it as flame.
    //    Otherwise, treat it as the wooden stick.

    // We define two colors:
    vec3 flameColor = vec3(1.0, 0.6, 0.0);   // bright orange flame
    vec3 stickColor = vec3(0.3, 0.2, 0.1);   // dark brown wood

    // We'll add an emissive glow factor to the flame:
    float emissiveStrength = 2.0; // tweak as desired

    vec3 finalColor;

    if (fragPos.y > flameCutoffY)
    {
        // Flame region
        vec3 emissive = flameColor * emissiveStrength;
        // Combine the normal lighting (totalLight) with flameColor,
        // then add the emissive glow so it doesn't depend on external lights.
        finalColor = (totalLight * flameColor) + emissive;
    }
    else
    {
        // Stick region
        finalColor = totalLight * stickColor;
    }

    // -----------------------------------
    // 5) Multiply final color by texture and set fragColor
    vec4 texColor = texture(texture1, textureCoord);
    fragColor = vec4(finalColor, 1.0) * texColor;
}


