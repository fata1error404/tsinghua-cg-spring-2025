#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>                  // library for loading OpenGL functions (like glClear or glViewport)
#include <GLFW/glfw3.h>                 // library for creating windows and handling input – mouse clicks, keyboard input, or window resizes
#include <glm/glm.hpp>                  // for basic vector and matrix mathematics functions
#include <glm/gtc/matrix_transform.hpp> // for matrix transformation functions
#include <glm/gtc/type_ptr.hpp>         // for matrix conversion to raw pointers (OpenGL compatibility with GLM)

#define STB_IMAGE_IMPLEMENTATION // define a STB_IMAGE_IMPLEMENTATION macro (to tell the compiler to include function implementations)
#include "stb_image.h"           // library for image loading
#include "shader.h"              // implementation of the graphics pipeline
#include "camera.h"              // implementation of the camera system
#include "fog.h"
#include "rain.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);

// define window size as a hyperparameter
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// create a Camera class instance with a specified position and default values for other parameters, to access its functionality
Camera ourCamera;

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

bool firstMouse = true;         // flag to check if the mouse movement is being processed for the first time
float lastX = SCR_WIDTH / 2.0;  // starting cursor position (x-axis)
float lastY = SCR_HEIGHT / 2.0; // starting cursor position (y-axis)

glm::vec3 skyboxScaleRatio = glm::vec3(1.0f, 1.0f, 1.4f); // skybox ratio control parameters (z, y, z) to maintain skybox proportions
float waterOffset = 0.0f;
float waterSpeed = 0.1f;
float skyboxReflectivity = 0.3f;

float freq = 100.0f; // how many waves per unit distance (higher = more ripples)
float speed = 1.0f;  // how fast waves move (higher = faster motion)
float amp = 0.2f;    // height of waves (higher = taller waves)

float fogStart = 20.0f;   // at this distance fog begins
float fogEnd = 60.0f;     // at this distance objects are 100% fogged
float fogDensity = 0.02f; // for exp‐fog variant
float seaLevel = 0.0f;    // world‐space Y of your water plane
float fogHeight = 5.0f;   // how tall your “ground fog” layer is

int main()
{
    // initialize and configure (use core profile mode and OpenGL v3.3)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Terrain Project (Ivan Yazykov)", NULL, NULL);

    // set OpenGL context and callback
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);   // register scroll callback
    glfwSetCursorPosCallback(window, mouse_callback); // register mouse callback

    // hide and lock the mouse cursor to the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // enable depth testing to ensure correct pixel rendering order in 3D space (depth buffer prevents incorrect overlaying and redrawing of objects)
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. SKYBOX
    // __________

    // unit cube (1 x 1 x 1)
    float skyboxVertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    unsigned int skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // images that form a skybox
    std::vector<std::string> faces{
        "data/skybox/SkyBox1.bmp",
        "data/skybox/SkyBox3.bmp",
        "data/skybox/SkyBox4.bmp",
        "data/skybox/SkyBox4.bmp",
        "data/skybox/SkyBox0.bmp",
        "data/skybox/SkyBox2.bmp",
    };

    unsigned int skyboxTexture = loadCubemap(faces);

    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    // correct skybox, so that the sun is a circle and we would be at the bottom
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, skyboxScaleRatio);
    // model = glm::translate(model, glm::vec3(0.0f, 0.9f, 0.0f));
    skyboxShader.use();
    skyboxShader.setMat4("model", model);

    // 2. WATER
    // _________

    const int GRID = 100;
    std::vector<float> waterVertices(GRID * GRID * 6 * 5);

    unsigned int waterVBO, waterVAO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int waterTexture;
    glGenTextures(1, &waterTexture);
    glBindTexture(GL_TEXTURE_2D, waterTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("data/water.bmp", &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    Shader waterShader("shaders/water.vs", "shaders/water.fs");

    waterShader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(200.0f, 1.0f, 200.0f));
    waterShader.setMat4("model", model);
    waterShader.setInt("waterTexture", 0);
    waterShader.setInt("skyboxTexture", 1);
    waterShader.setVec3("skyboxScaleRatio", skyboxScaleRatio);
    waterShader.setFloat("reflectivity", skyboxReflectivity);

    waterShader.setFloat("fogStart", fogStart);
    waterShader.setFloat("fogEnd", fogEnd);
    waterShader.setFloat("fogDensity", fogDensity);
    waterShader.setFloat("seaLevel", seaLevel);
    waterShader.setFloat("fogHeight", fogHeight);
    waterShader.setVec3("fogColor", fogColor);

    // 3. TERRAIN
    // ___________

    int x_size, z_size;
    data = stbi_load("data/heightmap.bmp", &x_size, &z_size, &nrChannels, STBI_grey);

    float xyScale = 0.05f;
    float yScale = 5.0f / 255.0f;

    std::vector<float> terrainVertices;

    for (int i = 0; i < z_size - 1; ++i)
    {
        for (int j = 0; j < x_size - 1; ++j)
        {
            float y0 = data[i * x_size + j] * yScale;
            float y1 = data[i * x_size + (j + 1)] * yScale;
            float y2 = data[(i + 1) * x_size + (j + 1)] * yScale;
            float y3 = data[(i + 1) * x_size + j] * yScale;

            float u0 = j / (float)(x_size - 1);
            float v0 = i / (float)(z_size - 1);
            float u1 = (j + 1) / (float)(x_size - 1);
            float v1 = (i + 1) / (float)(z_size - 1);

            // Triangle 1
            terrainVertices.push_back(j * xyScale);
            terrainVertices.push_back(y0);
            terrainVertices.push_back(i * xyScale);
            terrainVertices.push_back(u0);
            terrainVertices.push_back(v0);

            terrainVertices.push_back((j + 1) * xyScale);
            terrainVertices.push_back(y1);
            terrainVertices.push_back(i * xyScale);
            terrainVertices.push_back(u1);
            terrainVertices.push_back(v0);

            terrainVertices.push_back((j + 1) * xyScale);
            terrainVertices.push_back(y2);
            terrainVertices.push_back((i + 1) * xyScale);
            terrainVertices.push_back(u1);
            terrainVertices.push_back(v1);

            // Triangle 2
            terrainVertices.push_back(j * xyScale);
            terrainVertices.push_back(y0);
            terrainVertices.push_back(i * xyScale);
            terrainVertices.push_back(u0);
            terrainVertices.push_back(v0);

            terrainVertices.push_back((j + 1) * xyScale);
            terrainVertices.push_back(y2);
            terrainVertices.push_back((i + 1) * xyScale);
            terrainVertices.push_back(u1);
            terrainVertices.push_back(v1);

            terrainVertices.push_back(j * xyScale);
            terrainVertices.push_back(y3);
            terrainVertices.push_back((i + 1) * xyScale);
            terrainVertices.push_back(u0);
            terrainVertices.push_back(v1);
        }
    }

    stbi_image_free(data);

    unsigned int terrainVBO, terrainVAO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(float), terrainVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    unsigned int terrainTexture;
    glGenTextures(1, &terrainTexture);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);
    data = stbi_load("data/terrain.bmp", &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    unsigned int detailTexture;
    glGenTextures(1, &detailTexture);
    glBindTexture(GL_TEXTURE_2D, detailTexture);
    data = stbi_load("data/detail.bmp", &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    Shader terrainShader("shaders/terrain.vs", "shaders/terrain.fs");

    terrainShader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.4f, 0.0f));
    terrainShader.setMat4("model", model);
    terrainShader.setInt("terrainTexture", 0);
    terrainShader.setInt("detailTexture", 1);
    terrainShader.setInt("skyboxTexture", 2);
    terrainShader.setVec3("skyboxScaleRatio", skyboxScaleRatio);
    terrainShader.setFloat("reflectivity", 0.0f);

    // 4. WEATHER
    // ___________

    Fog fogEmitter(ourCamera, seaLevel);
    Rain rainEmitter(ourCamera, seaLevel);

    // game loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // handle keyboard input
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the color buffer (fill the screen with a clear color) and the depth buffer; otherwise the information of the previous frame stays in these buffers

        glm::mat4 view = ourCamera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(ourCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glDepthFunc(GL_LEQUAL); // ensure the skybox fail the depth test wherever there's a different object in front of it (its depth is set to 1.0 in the vertex shader, so we need less or equal depth function)

        // render skybox
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix so the skybox moves with the camera, creating the illusion of an infinitely distant environments
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // render water
        for (int i = 0; i < GRID; ++i)
        {
            for (int j = 0; j < GRID; ++j)
            {
                float x0 = -1.0f + 2.0f * j / GRID;
                float z0 = -1.0f + 2.0f * i / GRID;
                float x1 = -1.0f + 2.0f * (j + 1) / GRID;
                float z1 = -1.0f + 2.0f * (i + 1) / GRID;

                float h00 = sin(x0 * freq + currentFrame * speed) * amp + cos(z0 * freq + currentFrame * speed) * amp;
                float h10 = sin(x1 * freq + currentFrame * speed) * amp + cos(z0 * freq + currentFrame * speed) * amp;
                float h11 = sin(x1 * freq + currentFrame * speed) * amp + cos(z1 * freq + currentFrame * speed) * amp;
                float h01 = sin(x0 * freq + currentFrame * speed) * amp + cos(z1 * freq + currentFrame * speed) * amp;

                int base = (i * GRID + j) * 6 * 5;

                waterVertices[base + 0] = x0;
                waterVertices[base + 1] = h00;
                waterVertices[base + 2] = z0;
                waterVertices[base + 3] = 0.0f;
                waterVertices[base + 4] = 0.0f;

                waterVertices[base + 5] = x1;
                waterVertices[base + 6] = h10;
                waterVertices[base + 7] = z0;
                waterVertices[base + 8] = 1.0f;
                waterVertices[base + 9] = 0.0f;

                waterVertices[base + 10] = x1;
                waterVertices[base + 11] = h11;
                waterVertices[base + 12] = z1;
                waterVertices[base + 13] = 1.0f;
                waterVertices[base + 14] = 1.0f;

                waterVertices[base + 15] = x0;
                waterVertices[base + 16] = h00;
                waterVertices[base + 17] = z0;
                waterVertices[base + 18] = 0.0f;
                waterVertices[base + 19] = 0.0f;

                waterVertices[base + 20] = x1;
                waterVertices[base + 21] = h11;
                waterVertices[base + 22] = z1;
                waterVertices[base + 23] = 1.0f;
                waterVertices[base + 24] = 1.0f;

                waterVertices[base + 25] = x0;
                waterVertices[base + 26] = h01;
                waterVertices[base + 27] = z1;
                waterVertices[base + 28] = 0.0f;
                waterVertices[base + 29] = 1.0f;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, waterVertices.size() * sizeof(float), waterVertices.data());

        waterShader.use();
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);
        waterShader.setVec3("cameraPos", ourCamera.Position);

        waterOffset += waterSpeed * deltaTime;
        waterShader.setFloat("offset", waterOffset);

        glBindVertexArray(waterVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, waterVertices.size() / 3);
        glBindVertexArray(0);

        // render terrain
        terrainShader.use();
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", projection);
        terrainShader.setVec3("cameraPos", ourCamera.Position);

        glBindVertexArray(terrainVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, detailTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, terrainVertices.size() / 3);
        glBindVertexArray(0);

        // render fog
        // fogEmitter.draw(deltaTime, view, projection);
        // rainEmitter.draw(deltaTime, view, projection);

        glfwSwapBuffers(window); // make the contents of the back buffer (stores the completed frames) visible on the screen
        glfwPollEvents();        // if any events are triggered (like keyboard input or mouse movement events), updates the window state, and calls the corresponding functions (which we can register via callback methods)
    }

    // terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
}

// whenever the window size changed (by OS or user resize), this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// whenever the mouse uses scroll wheel, this callback function executes
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // handle mouse wheel scroll input using the Camera class function
    ourCamera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    // handle the first mouse movement to prevent a sudden jump
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // calculate the mouse offset since the last frame
    // (xpos and ypos are the current cursor coordinates in screen space)
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    // handle mouse movement input using the Camera class function
    ourCamera.ProcessMouseMovement(xoffset, yoffset);
}

// process all input: query GLFW whether relevant keys are pressed / released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    // Escape key to close the program
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    /*
       WASD keys for camera movement:
       W – move forward (along the camera's viewing direction vector, i.e. z-axis)
       S – move backward
       A – move left (along the right vector, i.e. x-axis; computed using the cross product)
       D – move right
    */
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(RIGHT);

    ourCamera.UpdatePosition(deltaTime);
}

// load and configure a cubemap texture for a skybox using six face images
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }

    return texture;
}