#version 400

in vec3 norm;
in vec3 fragPos;
in vec2 textureCoord;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float time;
uniform float amplitude;

void main()
{
    // Calculate wave-based normal
    float dx = 0.001;
    float centerVal = amplitude * sin((fragPos.x + fragPos.z) * 0.5 + time);
    float offsetXVal = amplitude * sin((fragPos.x + dx + fragPos.z) * 0.5 + time);
    float offsetZVal = amplitude * sin((fragPos.x + fragPos.z + dx) * 0.5 + time);

    float fx = (offsetXVal - centerVal) / dx;
    float fz = (offsetZVal - centerVal) / dx;

    vec3 tangentX = normalize(vec3(1.0, fx, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, fz, 1.0));
    vec3 waveNormal = normalize(cross(tangentZ, tangentX));

    // Lighting calculations (Phong)
    vec3 ambient = 0.3 * lightColor;

    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(waveNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, waveNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    // Combine with texture color
    vec4 waterColor = texture(texture1, textureCoord);
    fragColor = vec4(lighting, 1.0) * waterColor;
}
