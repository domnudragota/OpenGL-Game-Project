#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 textureCoord;
out vec3 norm;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 model;
uniform float time; // Current time for wave animation

// Function to calculate the wave height at a given position
float calculateWaveHeight(vec2 position, float time) {
    float amplitude = 3.0;               // Wave amplitude
    float frequency = 2.0 * 3.14159 / 10.0; // Wave frequency
    float phase = time;                  // Phase for time-based animation
    float direction = 1.0;               // Wave direction factor
    return amplitude * sin(direction * (position.x + position.y) * frequency + phase);
}

void main()
{
    // Modify the vertex position to simulate waves
    vec3 modifiedPos = pos;
    modifiedPos.y += calculateWaveHeight(pos.xz, time);

    // Pass texture coordinates and transformed normals
    textureCoord = texCoord;
    vec4 worldPos = model * vec4(modifiedPos, 1.0f);
    fragPos = vec3(worldPos);
    norm = mat3(transpose(inverse(model))) * normals;

    // Calculate the final vertex position
    gl_Position = MVP * vec4(modifiedPos, 1.0f);
}
