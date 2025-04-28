#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ctime>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera ourCamera(glm::vec3(0.0f, 6.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;

int currentTessLevel = 16; // tesselation level (defines the number of segments along each axis, i.e. how finely the surface is divided)
int currentMode = 0;       // rendering mode

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // tesselation requires using OpenGL 4.0+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 5 (Ivan Yazykov)", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    glPatchParameteri(GL_PATCH_VERTICES, 25); // set the number of control points per patch (primitive for tesselation)

    // generate 5 x 5 grid of control points with random height values
    std::srand(std::time(NULL));
    glm::vec3 controlPoints[5][5];

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            float y = (float(rand()) / float(RAND_MAX)) * 3.0f;
            controlPoints[i][j] = glm::vec3(i, y, j);
        }
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), controlPoints, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("texture.png", &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    Shader pointShader("shader vertex.glsl", "shader fragment.glsl");

    // adjusted graphics pipeline for tesselation (shaders: vertex → TCS → TES → fragment)
    Shader surfaceShader("shader vertex.glsl", "shader TCS.glsl", "shader TES.glsl", "shader fragment.glsl");

    surfaceShader.use();
    surfaceShader.setInt("renderMode", currentMode);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw Bezier surface
        surfaceShader.use();
        glm::mat4 view = ourCamera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(ourCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        surfaceShader.setMat4("view", view);
        surfaceShader.setMat4("projection", projection);
        surfaceShader.setInt("tessLevel", currentTessLevel);
        surfaceShader.setInt("renderMode", currentMode);

        if (currentMode == 0)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(VAO);
        glDrawArrays(GL_PATCHES, 0, 25); // draw 25 vertices using the patch primitive (since 1 patch = 25 control points, this is just 1 patch)

        // draw control points

        /*
            pointShader.use();
            pointShader.setMat4("view", view);
            pointShader.setMat4("projection", projection);
            pointShader.setInt("isRenderControlPoints", 1);
            glPointSize(5.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDrawArrays(GL_POINTS, 0, 25);
        */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    ourCamera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    ourCamera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ourCamera.ProcessKeyboard(RIGHT, deltaTime);

    // surface smoothness control
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        currentTessLevel += 1;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        currentTessLevel -= 1;

    if (currentTessLevel < 3)
        currentTessLevel = 3;
    if (currentTessLevel > 64)
        currentTessLevel = 64;

    // render mode control
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        currentMode = 0;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        currentMode = 1;
}