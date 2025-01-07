#version 400

in vec3 fragPos;
in vec3 norm;
in vec2 textureCoord;

out vec4 fragColor;

uniform sampler2D diffuseMap;
uniform sampler2D emissionMap;
uniform vec3 torchColor;
uniform vec3 torchPos;
uniform float flameCutoffY;

void main()
{
    vec4 diffuseTex = texture(diffuseMap, textureCoord);
    vec4 emissionTex = texture(emissionMap, textureCoord);

    // Flame region: use emission texture
    if (fragPos.y > flameCutoffY)
    {
        fragColor = emissionTex;
    }
    else // Stick region: use diffuse texture
    {
        fragColor = diffuseTex;
    }
}
