#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "sphere.h"
#include "font.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window, Shader &ourShader);

struct PlanetData
{
    std::string name;
    float radius;
    glm::vec3 orbit;
    float orbitSpeed;
    float angle;
};

// 1.1 set planet sizes and orbits, using real solar system data (scaled for rendering, the Sun is at the origin with no orbit translation)
std::vector<PlanetData> data = {
    {"Sun", 15, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f},
    {"Mercury", 1.5, glm::vec3(1.0f, 0.5f, 0.0f), 0.02f, 0.0f},
    {"Venus", 2.5, glm::vec3(1.4f, 0.7f, 0.0f), 0.01f, 0.0f},
    {"Earth", 3.0, glm::vec3(1.9f, 1.0f, 0.0f), 0.005f, 0.0f},
    {"Moon", 0.8, glm::vec3(0.2f, 0.2f, 0.0f), 0.05f, 0.0f},
    {"Mars", 1.5, glm::vec3(2.1f, 1.0f, 0.0f), 0.004f, 0.0f},
    {"Jupiter", 10.0, glm::vec3(5.8f, 2.5f, 0.0f), 0.002f, 0.0f},
    {"Saturn", 9.0, glm::vec3(9.58f, 4.0f, 0.0f), 0.0015f, 0.0f},
    {"Uranus", 6.5, glm::vec3(19.20f, 8.0f, 0.0f), 0.0008f, 0.0f},
    {"Neptune", 6.5, glm::vec3(30.7f, 12.0f, 0.0f), 0.0005f, 0.0f}};

glm::vec3 earthPos;
float controlSpeed = 1.0f;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera ourCamera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Assignment 2 (Ivan Yazykov)", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Sphere ourSphere(36, 18);

    Shader sphereShader("sphere shader.vs", "sphere shader.fs");

    Shader fontShader("font shader.vs", "font shader.fs");

    // 1.3 load font
    FontRenderer font;
    font.Load("Arial.ttf", 48);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, sphereShader);

        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphereShader.use();
        glm::mat4 view = ourCamera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(ourCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        sphereShader.setMat4("view", view);
        sphereShader.setMat4("projection", projection);

        for (unsigned int i = 0, n = data.size(); i < n; i++)
        {
            data[i].angle += data[i].orbitSpeed * controlSpeed;

            if (data[i].angle > 2 * glm::pi<float>())
                data[i].angle -= 2 * glm::pi<float>();

            glm::vec3 orbitPos;
            orbitPos.x = data[i].orbit.x * cos(data[i].angle);
            orbitPos.z = data[i].orbit.y * sin(data[i].angle);
            orbitPos.y = 0.0f;

            if (data[i].name == "Earth")
                earthPos = orbitPos;
            else if (data[i].name == "Moon")
            {
                float moonOrbitRadius = 0.5f;
                orbitPos.x = earthPos.x + moonOrbitRadius * cos(data[i].angle);
                orbitPos.z = earthPos.z + moonOrbitRadius * sin(data[i].angle);
                orbitPos.y = 0.0f;
            }

            ourSphere.drawSphere(data[i].radius * 0.02, orbitPos, "texture/" + data[i].name + ".jpg", sphereShader, fontShader, data[i].name, font, ourCamera, controlSpeed);
        }

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

void processInput(GLFWwindow *window, Shader &ourShader)
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

    // 2. keyboard control for rotation speed
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        controlSpeed *= 0.95f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        controlSpeed *= 1.05f;
}