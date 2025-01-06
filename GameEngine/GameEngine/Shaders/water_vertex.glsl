#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 textureCoord;
out vec3 norm;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 model;
uniform float time; // We'll pass glfwGetTime() here

void main()
{
    float Ai   = 3.0;              // Wave amplitude factor
    float Wi   = 2.0 * 3.14159 / 10.0;  // Wave frequency
    float Di   = 1.0;             // Wave direction factor
    float phi  = time;            // Phase, changes with time

    // Make a copy of 'pos' and displace Y
    vec3 modifiedPos = pos;
    modifiedPos.y += 2.0 * Ai *
        (sin(Di * (pos.x + pos.z) * Wi + phi) + 1.0) / 2.0;

    // Same logic for passing texture & normal as usual
    textureCoord = texCoord;
    vec4 worldPos = model * vec4(modifiedPos, 1.0f);
    fragPos = vec3(worldPos);
    norm = mat3(transpose(inverse(model))) * normals;

    // Final position
    gl_Position = MVP * vec4(modifiedPos, 1.0f);
}
