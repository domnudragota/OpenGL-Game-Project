#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"

// Global Variables
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Window + camera
Window window("Game Engine", 1024, 768);
Camera camera;

// Light properties
glm::vec3 sunColor = glm::vec3(1.0f); // Sunlight
glm::vec3 sunPos = glm::vec3(-180.0f, 100.0f, -200.0f);

glm::vec3 torchColor = glm::vec3(1.0f, 0.5f, 0.1f); // Torchlight
glm::vec3 torchPos; // To be updated with the first box position
float torchAmbientStrength = 0.3f;
float torchSpecularStrength = 1.0f;

// Boat properties
glm::vec3 boatPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 boatDirection = glm::vec3(0.0f, 0.0f, -1.0f);
float boatAngle = 0.0f;

// First box properties
glm::vec3 box1Position = glm::vec3(10.0f, 0.0f, -20.0f); // Initial position for the first box

// Second box properties
glm::vec3 box2Position = glm::vec3(-30.0f, 0.0f, -40.0f); // Initial position for the second box

// Dino properties
glm::vec3 dinoOffset = glm::vec3(0.0f, 2.5f, 0.0f); // Dino position relative to the boat
glm::vec3 dinoBoxOffset = glm::vec3(0.0f, 4.5f, 0.0f); // Dino position relative to the second box

// Tree properties
glm::vec3 treePosition = glm::vec3(20.0f, -20.0f, -50.0f); // Fixed position for the tree

// Apple properties
std::vector<glm::vec3> applePositions = {
    glm::vec3(21.0f, -19.0f, -49.0f),
    glm::vec3(19.5f, -19.2f, -50.5f),
    glm::vec3(20.0f, -19.1f, -51.0f),
    glm::vec3(20.5f, -19.0f, -49.5f),
    glm::vec3(18.5f, -19.3f, -50.0f)
};

// Function Prototypes
void processKeyboardInput(glm::vec3& boatPosition, glm::vec3& boatDirection, float& boatAngle, float deltaTime);
void updateSunPosition(float currentFrame);
float calculateWaveHeight(float x, float z, float time);

int main() {
    glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

    // Build and compile shaders
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
    Shader waterShader("Shaders/water_vertex.glsl", "Shaders/water_fragment.glsl");

    // Torch Shader
    Shader torchShader("Shaders/torch_vertex.glsl", "Shaders/torch_fragment.glsl");

    // Load textures
    GLuint tex = loadBMP("Resources/Textures/wood.bmp");
    GLuint tex2 = loadBMP("Resources/Textures/blue.bmp");
    GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");

    // Torch textures
    GLuint diffuseTex = loadBMP("Resources/Textures/Diffuse.bmp");
    GLuint emissionTex = loadBMP("Resources/Textures/Emission.bmp");

    // Dino texture
    GLuint dinoTex = loadBMP("Resources/Textures/dino-0.bmp");

    glEnable(GL_DEPTH_TEST);

    // Load meshes
    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh box = loader.loadObj("Resources/Models/cube.obj", { Texture{tex, "texture_diffuse"} });
    Mesh plane = loader.loadObj("Resources/Models/plane1.obj", { Texture{tex2, "texture_diffuse"} });
    Mesh boat = loader.loadObj("Resources/Models/boat.obj", { Texture{tex, "texture_diffuse"} });
    Mesh torch = loader.loadObj("Resources/Models/Torch.obj",
        { Texture{diffuseTex, "texture_diffuse"},
          Texture{emissionTex, "texture_emission"} });
    Mesh dino = loader.loadObj("Resources/Models/dino-0.obj", { Texture{dinoTex, "texture_diffuse"} });
    Mesh tree = loader.loadObj("Resources/Models/tree.obj"); // No texture applied
    Mesh apple = loader.loadObj("Resources/Models/sphere.obj", { Texture{tex3, "texture_diffuse"} });

    while (!window.isPressed(GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window.getWindow())) {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput(boatPosition, boatDirection, boatAngle, deltaTime);
        updateSunPosition(currentFrame);

        // Calculate wave height for the first box and adjust its position
        float box1WaveHeight = calculateWaveHeight(box1Position.x, box1Position.z, currentFrame);
        glm::vec3 adjustedBox1Position = box1Position;
        adjustedBox1Position.y = box1WaveHeight - 10.0f;

        // Update torch position based on the first box
        torchPos = adjustedBox1Position + glm::vec3(0.0f, 5.0f, 0.0f); // Offset torch above the first box

        // Calculate wave height for the second box and adjust its position
        float box2WaveHeight = calculateWaveHeight(box2Position.x, box2Position.z, currentFrame);
        glm::vec3 adjustedBox2Position = box2Position;
        adjustedBox2Position.y = box2WaveHeight - 10.0f;

        // Prepare projection and view matrices
        glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
        glm::mat4 ViewMatrix = glm::lookAt(
            boatPosition + glm::vec3(0.0f, 5.0f, 10.0f), // Camera position relative to boat
            boatPosition,                                // Camera looks at the boat
            camera.getCameraUp());                      // Camera's up vector

        // Render the sun
        sunShader.use();
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, sunPos);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(sunShader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        sun.draw(sunShader);

        // Render apples around the tree
        shader.use();
        glUniform3f(glGetUniformLocation(shader.getId(), "sunColor"), sunColor.x, sunColor.y, sunColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "sunPos"), sunPos.x, sunPos.y, sunPos.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), boatPosition.x, boatPosition.y, boatPosition.z);

        for (const glm::vec3& pos : applePositions) {
            ModelMatrix = glm::mat4(1.0f);
            ModelMatrix = glm::translate(ModelMatrix, pos);
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1f)); // Scale down apples
            MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
            apple.draw(shader);
        }

        // Render the first floating box
        shader.use();
        glUniform3f(glGetUniformLocation(shader.getId(), "sunColor"), sunColor.x, sunColor.y, sunColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "sunPos"), sunPos.x, sunPos.y, sunPos.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), boatPosition.x, boatPosition.y, boatPosition.z);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, adjustedBox1Position);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        box.draw(shader);

        // Render the second floating box
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, adjustedBox2Position);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        box.draw(shader);

        // Render the torch
        torchShader.use();
        glUniform3f(glGetUniformLocation(torchShader.getId(), "torchColor"), torchColor.x, torchColor.y, torchColor.z);
        glUniform3f(glGetUniformLocation(torchShader.getId(), "torchPos"), torchPos.x, torchPos.y, torchPos.z);
        glUniform1f(glGetUniformLocation(torchShader.getId(), "torchAmbientStrength"), torchAmbientStrength);
        glUniform1f(glGetUniformLocation(torchShader.getId(), "torchSpecularStrength"), torchSpecularStrength);
        glUniform3f(glGetUniformLocation(torchShader.getId(), "viewPos"), boatPosition.x, boatPosition.y, boatPosition.z);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, torchPos);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(torchShader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(torchShader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        torch.draw(torchShader);

        // Render the dinosaur on the boat
        shader.use();
        glm::vec3 dinoPosition = boatPosition + dinoOffset; // Dino relative to the boat
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, dinoPosition);
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(boatAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate with the boat
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        dino.draw(shader);

        // Render the dinosaur on the second box
        glm::vec3 dinoBoxPosition = adjustedBox2Position + dinoBoxOffset; // Dino relative to the second box
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, dinoBoxPosition);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        dino.draw(shader);

        // Render the tree
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, treePosition);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        tree.draw(shader);

        // Render the water plane
        waterShader.use();
        glUniform1f(glGetUniformLocation(waterShader.getId(), "time"), currentFrame);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "lightColor"), sunColor.x, sunColor.y, sunColor.z);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "lightPos"), sunPos.x, sunPos.y, sunPos.z);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "viewPos"), boatPosition.x, boatPosition.y, boatPosition.z);

        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(waterShader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(waterShader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        plane.draw(waterShader);

        // Render the boat
        shader.use();
        float waveHeight = calculateWaveHeight(boatPosition.x, boatPosition.z, currentFrame);
        boatPosition.y = waveHeight - 4.5f;

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, boatPosition);
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(boatAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        boat.draw(shader);

        window.update();
    }

    return 0;
}

void processKeyboardInput(glm::vec3& boatPosition, glm::vec3& boatDirection, float& boatAngle, float deltaTime) {
    float boatSpeed = 20.0f * deltaTime;
    float rotationSpeed = 120.0f * deltaTime;

    if (window.isPressed(GLFW_KEY_W))
        boatPosition += boatDirection * boatSpeed; // Forward
    if (window.isPressed(GLFW_KEY_S))
        boatPosition -= boatDirection * boatSpeed; // Backward

    if (window.isPressed(GLFW_KEY_A)) {
        boatAngle += rotationSpeed; // Rotate left
        boatDirection = glm::vec3(cos(glm::radians(boatAngle)), 0.0f, sin(glm::radians(boatAngle)));
    }
    if (window.isPressed(GLFW_KEY_D)) {
        boatAngle -= rotationSpeed; // Rotate right
        boatDirection = glm::vec3(cos(glm::radians(boatAngle)), 0.0f, sin(glm::radians(boatAngle)));
    }

    boatPosition.x = glm::clamp(boatPosition.x, -200.0f, 200.0f);
    boatPosition.z = glm::clamp(boatPosition.z, -200.0f, 200.0f);
}

void updateSunPosition(float currentFrame) {
    float scaledTime = currentFrame * 0.1f;
    sunPos.x = 150.0f * sin(scaledTime);
    sunPos.z = 150.0f * cos(scaledTime);
    sunPos.y = 100.0f;
}

float calculateWaveHeight(float x, float z, float time) {
    return 3.0f * sin(2.0f * x + time) * cos(2.0f * z + time);
}
