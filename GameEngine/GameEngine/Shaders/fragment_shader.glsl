#version 400

in vec2 textureCoord; 
in vec3 norm;       // original normal passed in (we won't remove it)
in vec3 fragPos;

out vec4 fragColor;

// Your existing uniforms
uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

// NEW UNIFORMS for wave-based normal calculation
uniform float time;       // same time you pass to the vertex shader
uniform float amplitude;  // wave amplitude you want to use

// OPTIONAL: If your wave frequency or speed is in the vertex shader, you can unify them here too.
// e.g. uniform float frequency; etc.

// 1) Define a wave function that returns y = wave height at (x,z) + time.
float waveFunc(vec2 xz, float t)
{
    // Example: simple sine wave. You can adjust to match your vertex logic.
    // amplitude * sin( (x+z)*freq + t )
    float freq = 0.5; // or whatever frequency you use in vertex
    return amplitude * sin((xz.x + xz.y) * freq + t);
}

void main()
{
    // 2) Approximate partial derivatives of the wave function
    // We'll do a small offset (dx) in x and z to find slope.
    float dx = 0.001;

    // current wave height at fragPos.xz
    float centerVal = waveFunc(fragPos.xz, time);

    // wave at slightly offset x
    float offsetXVal = waveFunc(fragPos.xz + vec2(dx, 0.0), time);
    // wave at slightly offset z
    float offsetZVal = waveFunc(fragPos.xz + vec2(0.0, dx), time);

    // partial derivatives (slopes) in x and z
    float fx = (offsetXVal - centerVal) / dx; 
    float fz = (offsetZVal - centerVal) / dx;

    // 3) Build tangent vectors from these slopes
    // tangentX is the direction if we move in +X on the surface
    vec3 tangentX = normalize(vec3(1.0, fx, 0.0));
    // tangentZ is the direction if we move in +Z on the surface
    vec3 tangentZ = normalize(vec3(0.0, fz, 1.0));

    // cross(tangentZ, tangentX) = normal pointing "up" from wave slope
    vec3 waveNormal = normalize(cross(tangentZ, tangentX));

    // 4) Use waveNormal in the Phong lighting instead of 'norm'
    // Original code: (unchanged lines except we replace 'normal' with 'waveNormal')

    // Ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(waveNormal, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.7;
    vec3 viewDir    = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, waveNormal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular   = specularStrength * spec * lightColor;

    // Combine lighting
    vec3 result = ambient + diffuse + specular;

    // Multiply by your texture
    fragColor = vec4(result, 1.0f) * texture(texture1, textureCoord);
}
