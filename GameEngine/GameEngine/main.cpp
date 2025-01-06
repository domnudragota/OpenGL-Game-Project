#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"

void processKeyboardInput();

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// Window + camera
Window window("Game Engine", 800, 800);
Camera camera;

// Light properties
glm::vec3 lightColor = glm::vec3(1.0f); //let the sun shine
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

int main()
{
    glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

    // Build and compile shaders
    // 1) Original shader (no wave) for the box
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    // 2) Sun shader for the light sphere
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
    // 3) Water shader (with wave logic in the vertex shader) for the plane
    Shader waterShader("Shaders/water_vertex.glsl", "Shaders/fragment_shader.glsl");

    // Load some textures
    GLuint tex = loadBMP("Resources/Textures/wood.bmp");
    GLuint tex2 = loadBMP("Resources/Textures/blue.bmp");
    GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");

    glEnable(GL_DEPTH_TEST);

    // A small quad mesh (not essential, just left in from your original code)
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

    vert[0].normals = glm::normalize(glm::cross(
        vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
    vert[1].normals = glm::normalize(glm::cross(
        vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
    vert[2].normals = glm::normalize(glm::cross(
        vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
    vert[3].normals = glm::normalize(glm::cross(
        vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

    std::vector<int> ind = { 0,1,3, 1,2,3 };

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

    Mesh mesh(vert, ind, textures3);

    // Load the sphere for the sun, the box, and the plane
    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
    Mesh plane = loader.loadObj("Resources/Models/plane1.obj", textures2);

    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

        {
            float scaledTime = currentFrame * 0.1f;  // revolve slower
            lightPos.x = 150.0f * sin(scaledTime);
            lightPos.z = 150.0f * cos(scaledTime);
            lightPos.y = 100.0f; // keep same height
        }

        // 1) Draw the sun (light source) using sunShader
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

        sun.draw(sunShader);

        // 2) Draw the box with the ORIGINAL shader (no wave)
        shader.use();

        // Pass lighting uniforms (for the fragment shader)
        glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"),
            lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"),
            lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"),
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Setup MVP for the box
        GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
        GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

        ModelMatrix = glm::mat4(1.0f);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

        box.draw(shader);

        // 3) Draw the plane with the WATER shader (wave logic)
        waterShader.use();

        // Same lighting uniforms for the water fragment shader
        glUniform3f(glGetUniformLocation(waterShader.getId(), "lightColor"),
            lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "lightPos"),
            lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "viewPos"),
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Pass time for the wave vertex shader
        glUniform1f(glGetUniformLocation(waterShader.getId(), "time"), currentFrame);

        // If your water shader uses amplitude/waterColor, set them
        glUniform1f(glGetUniformLocation(waterShader.getId(), "amplitude"), 1.0f);
        glUniform3f(glGetUniformLocation(waterShader.getId(), "waterColor"),
            0.0f, 0.4f, 1.0f);

        // Setup MVP for the plane
        GLuint waterMVPID = glGetUniformLocation(waterShader.getId(), "MVP");
        GLuint waterModelID = glGetUniformLocation(waterShader.getId(), "model");

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f)); // place lower
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(waterMVPID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(waterModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

        // Draw the plane (now wavy)
        plane.draw(waterShader);

        window.update();
    }

    return 0;
}

void processKeyboardInput()
{
    float cameraSpeed = 30 * deltaTime;

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
