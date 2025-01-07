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
glm::vec3 torchLightColor = glm::vec3(10.0f, 6.0f, 3.0f); // Strong orange

int main()
{
    glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

    // Build and compile shaders
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
    Shader torchShader("Shaders/torch_vertex.glsl", "Shaders/torch_fragment.glsl");

    // Textures
    GLuint tex = loadBMP("Resources/Textures/wood.bmp");
    GLuint tex2 = loadBMP("Resources/Textures/rock.bmp"); // not used, but loaded
    GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");

    GLuint torchDiffuse = loadBMP("Resources/Textures/Diffuse.bmp");
    GLuint torchEmission = loadBMP("Resources/Textures/Emission.bmp");

    // --- Separate texture arrays ---
    std::vector<Texture> boxTextures;
    boxTextures.push_back(Texture());
    boxTextures[0].id = tex;  // wood.bmp
    boxTextures[0].type = "texture_diffuse";

    std::vector<Texture> planeTextures;
    planeTextures.push_back(Texture());
    planeTextures[0].id = tex3; // orange.bmp
    planeTextures[0].type = "texture_diffuse";

    std::vector<Texture> torchTextures;
    torchTextures.push_back(Texture());
    torchTextures[0].id = torchDiffuse;
    torchTextures[0].type = "texture_diffuse";

    torchTextures.push_back(Texture());
    torchTextures[1].id = torchEmission;
    torchTextures[1].type = "texture_emission";

    glEnable(GL_DEPTH_TEST);

    // Load OBJ models
    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh box = loader.loadObj("Resources/Models/cube.obj", boxTextures);
    Mesh plane = loader.loadObj("Resources/Models/plane.obj", planeTextures);
    Mesh torch = loader.loadObj("Resources/Models/Torch.obj", torchTextures);

    // Rendering loop
    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

        // Move the sun in a slow circle
        float scaledTime = glfwGetTime() * 0.10f;
        lightPos.x = 150.0f * sin(scaledTime);
        lightPos.z = 150.0f * cos(scaledTime);
        lightPos.y = 100.0f;

        // --- 1) Render the Sun ---
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

        // Draw the sun geometry
        sun.draw(sunShader);

        // --- 2) Render the Plane (uses main shader) ---
        shader.use();

        // Pass the sun uniforms
        glUniform3f(glGetUniformLocation(shader.getId(), "sunColor"),
            lightColor.x, lightColor.y, lightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "sunPos"),
            lightPos.x, lightPos.y, lightPos.z);

        // Pass the torch as a second light
        glUniform3f(glGetUniformLocation(shader.getId(), "torchColor"),
            torchLightColor.x, torchLightColor.y, torchLightColor.z);
        glUniform3f(glGetUniformLocation(shader.getId(), "torchPos"),
            torchLightPos.x, torchLightPos.y, torchLightPos.z);

        // Camera info
        glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"),
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        // Ambient strength
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.3f);

        GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
        GLuint ModelMatrixID2 = glGetUniformLocation(shader.getId(), "model");

        // Bind orange texture for the plane
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex3); // Orange texture
        glUniform1i(glGetUniformLocation(shader.getId(), "texture1"), 0);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -20.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID2, 1, GL_FALSE, &ModelMatrix[0][0]);

        plane.draw(shader);

        // --- 3) Render the Box (still uses main shader) ---
        // We already "shader.use()" above, so no need to do it again

        // Bind wood texture for the box
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex); // wood texture
        glUniform1i(glGetUniformLocation(shader.getId(), "texture1"), 0);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID2, 1, GL_FALSE, &ModelMatrix[0][0]);

        box.draw(shader);

        // --- 4) Render the Torch (uses torch shader) ---
        torchShader.use();
        glUniform3f(glGetUniformLocation(torchShader.getId(), "torchColor"),
            torchLightColor.x, torchLightColor.y, torchLightColor.z);
        glUniform3f(glGetUniformLocation(torchShader.getId(), "torchPos"),
            torchLightPos.x, torchLightPos.y, torchLightPos.z);
        glUniform3f(glGetUniformLocation(torchShader.getId(), "viewPos"),
            camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
        glUniform1f(glGetUniformLocation(torchShader.getId(), "flameCutoffY"), -13.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, torchDiffuse);
        glUniform1i(glGetUniformLocation(torchShader.getId(), "diffuseMap"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, torchEmission);
        glUniform1i(glGetUniformLocation(torchShader.getId(), "emissionMap"), 1);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, torchLightPos);
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(torchShader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(torchShader.getId(), "model"), 1, GL_FALSE, &ModelMatrix[0][0]);
        torch.draw(torchShader);

        // Update the window
        window.update();
    }

    return 0;
}

void processKeyboardInput()
{
    float cameraSpeed = 90 * deltaTime;
    if (window.isPressed(GLFW_KEY_W)) camera.keyboardMoveFront(cameraSpeed);
    if (window.isPressed(GLFW_KEY_S)) camera.keyboardMoveBack(cameraSpeed);
    if (window.isPressed(GLFW_KEY_A)) camera.keyboardMoveLeft(cameraSpeed);
    if (window.isPressed(GLFW_KEY_D)) camera.keyboardMoveRight(cameraSpeed);
    if (window.isPressed(GLFW_KEY_R)) camera.keyboardMoveUp(cameraSpeed);
    if (window.isPressed(GLFW_KEY_F)) camera.keyboardMoveDown(cameraSpeed);

    if (window.isPressed(GLFW_KEY_LEFT))  camera.rotateOy(cameraSpeed);
    if (window.isPressed(GLFW_KEY_RIGHT)) camera.rotateOy(-cameraSpeed);
    if (window.isPressed(GLFW_KEY_UP))    camera.rotateOx(cameraSpeed);
    if (window.isPressed(GLFW_KEY_DOWN))  camera.rotateOx(-cameraSpeed);
}
