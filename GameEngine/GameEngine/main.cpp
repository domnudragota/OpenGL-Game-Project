#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"

void processKeyboardInput();

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

Window window("Game Engine", 800, 800);
Camera camera;

// Original sun properties
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

// Torch light properties
glm::vec3 torchLightPos = glm::vec3(0.0f, -15.0f, 10.0f);
glm::vec3 torchLightColor = glm::vec3(10.0f, 6.0f, 3.0f); // strong orange

int main()
{
    glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

    // Build and compile shaders
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    // Textures
    GLuint tex = loadBMP("Resources/Textures/wood.bmp");
    GLuint tex2 = loadBMP("Resources/Textures/rock.bmp");
    GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");

    glEnable(GL_DEPTH_TEST);

    // Create a simple quad mesh (not used later, but kept for reference)
    std::vector<Vertex> vert;
    vert.push_back(Vertex());
    vert[0].pos = glm::vec3(10.5f, 10.5f, 0.0f);
    vert[0].textureCoords = glm::vec2(1.0f, 1.0f);

    vert.push_back(Vertex());
    vert[1].pos = glm::vec3(10.5f, -10.5f, 0.0f);
    vert[1].textureCoords = glm::vec2(1.0f, 0.0f);

    vert.push_back(Vertex());
    vert[2].pos = glm::vec3(-10.5f, -10.5f, 0.0f);
    vert[2].textureCoords = glm::vec2(0.0f, 0.0f);

    vert.push_back(Vertex());
    vert[3].pos = glm::vec3(-10.5f, 10.5f, 0.0f);
    vert[3].textureCoords = glm::vec2(0.0f, 1.0f);

    // Simple normal calculation
    vert[0].normals = glm::normalize(glm::cross(vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
    vert[1].normals = glm::normalize(glm::cross(vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
    vert[2].normals = glm::normalize(glm::cross(vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
    vert[3].normals = glm::normalize(glm::cross(vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

    std::vector<int> ind = { 0, 1, 3, 1, 2, 3 };

    std::vector<Texture> textures;
    textures.push_back(Texture());
    textures[0].id = tex;
    textures[0].type = "texture_diffuse";

    std::vector<Texture> textures2;
    textures2.push_back(Texture());
    textures2[0].id = tex2;
    textures2[0].type = "texture_diffuse";

    std::vector<Texture> textures3;
    textures3.push_back(Texture());
    textures3[0].id = tex3;
    textures3[0].type = "texture_diffuse";


    
    GLuint torchDiffuse = loadBMP("Resources/Textures/Diffuse.bmp");
    GLuint torchEmission = loadBMP("Resources/Textures/Emission.bmp");

    std::vector<Texture> torchTextures;
    torchTextures.push_back(Texture());
    torchTextures[0].id = torchDiffuse;
    torchTextures[0].type = "texture_diffuse";

    torchTextures.push_back(Texture());
    torchTextures[1].id = torchEmission;
    torchTextures[1].type = "texture_emission";



    // Create a mesh from these vertices (not used below, just for reference)
    Mesh mesh(vert, ind, textures3);

    // Load OBJ models
    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
    Mesh plane = loader.loadObj("Resources/Models/plane.obj", textures3);
    // Torch (single mesh: top flame + stick in one)
    Mesh torch = loader.loadObj("Resources/Models/Torch.obj"); // Minecraft-style torch
    


    // Main loop
    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Camera input
        processKeyboardInput();

        // Move the sun in a slow circle
        float scaledTime = glfwGetTime() * 0.10f;
        lightPos.x = 150.0f * sin(scaledTime);
        lightPos.z = 150.0f * cos(scaledTime);
        lightPos.y = 100.0f;

        // --- 1) Render the Sun "visual" (the sphere) ---
        sunShader.use();

        glm::mat4 ProjectionMatrix = glm::perspective(
            90.0f,
            window.getWidth() * 1.0f / window.getHeight(),
            0.1f,
            10000.0f
        );

        glm::mat4 ViewMatrix = glm::lookAt(
            camera.getCameraPosition(),
            camera.getCameraPosition() + camera.getCameraViewDirection(),
            camera.getCameraUp()
        );

        GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, lightPos);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Draw sun geometry
        sun.draw(sunShader);

        // --- 2) Render the rest of the scene with both lights (sun + torch) ---
        shader.use();

        // Pass both lights + camera position to the same fragment shader
        glUniform3f(glGetUniformLocation(shader.getId(), "sunColor"), lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "sunPos"), lightPos.x, lightPos.y, lightPos.z);

        glUniform3f(glGetUniformLocation(shader.getId(), "torchColor"), torchLightColor.x, torchLightColor.y, torchLightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "torchPos"), torchLightPos.x, torchLightPos.y, torchLightPos.z);

        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"),
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // -- Add: uniform for splitting flame vs. stick by Y --
        // Adjust the cutoffY to match your torch's actual flame height
        glUniform1f(glGetUniformLocation(shader.getId(), "flameCutoffY"), -13.0f);

        // Get the matrix uniform locations once
        GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
        GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

        // --- Torch model ---
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, torchLightPos);
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        torch.draw(shader);

        // --- Box ---
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        box.draw(shader);

        // --- Plane ---
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        plane.draw(shader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, torchDiffuse);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, torchEmission);

        // Then set shader uniforms if needed:
        glUniform1i(glGetUniformLocation(shader.getId(), "diffuseMap"), 0);
        glUniform1i(glGetUniformLocation(shader.getId(), "emissionMap"), 1);

        // Finally draw the torch
        torch.draw(shader);

        window.update();
    }
    return 0;
}

// Same keyboard processing as before:
void processKeyboardInput()
{
    float cameraSpeed = 90 * deltaTime;

    // Translation
    if (window.isPressed(GLFW_KEY_W))
        camera.keyboardMoveFront(cameraSpeed);
    if (window.isPressed(GLFW_KEY_S))
        camera.keyboardMoveBack(cameraSpeed);
    if (window.isPressed(GLFW_KEY_A))
        camera.keyboardMoveLeft(cameraSpeed);
    if (window.isPressed(GLFW_KEY_D))
        camera.keyboardMoveRight(cameraSpeed);
    if (window.isPressed(GLFW_KEY_R))
        camera.keyboardMoveUp(cameraSpeed);
    if (window.isPressed(GLFW_KEY_F))
        camera.keyboardMoveDown(cameraSpeed);

    // Rotation
    if (window.isPressed(GLFW_KEY_LEFT))
        camera.rotateOy(cameraSpeed);
    if (window.isPressed(GLFW_KEY_RIGHT))
        camera.rotateOy(-cameraSpeed);
    if (window.isPressed(GLFW_KEY_UP))
        camera.rotateOx(cameraSpeed);
    if (window.isPressed(GLFW_KEY_DOWN))
        camera.rotateOx(-cameraSpeed);
}
