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
#include "light.h"
#include "weather rain.h"
#include "weather fog.h"
#include "1 skybox.h"
#include "2 water.h"
#include "3 terrain.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);

// window settings
bool isFullscreen = false;
const unsigned int DEFAULT_SCR_WIDTH = 800;
const unsigned int DEFAULT_SCR_HEIGHT = 600;
const unsigned int DEFAULT_WINDOW_POS_X = 100;
const unsigned int DEFAULT_WINDOW_POS_Y = 100;

unsigned int currentScreenWidth = DEFAULT_SCR_WIDTH;
unsigned int currentScreenHeight = DEFAULT_SCR_HEIGHT;

// shadow map resolution
const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

// create a Camera class instance with a specified position and default values for other parameters, to access its functionality
Camera ourCamera;

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

bool firstMouse = true;                 // flag to check if the mouse movement is being processed for the first time
float lastX = DEFAULT_SCR_WIDTH / 2.0;  // starting cursor position (x-axis)
float lastY = DEFAULT_SCR_HEIGHT / 2.0; // starting cursor position (y-axis)

bool showLighting = true;
bool showWeather = false;

int main()
{
    // initialize and configure (use core profile mode and OpenGL v3.3)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW window creation
    GLFWwindow *window = glfwCreateWindow(DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT, "Terrain Project (Ivan Yazykov)", NULL, NULL);
    glfwSetWindowPos(window, DEFAULT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y);

    // set OpenGL context and callback
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);   // register scroll callback
    glfwSetCursorPosCallback(window, mouse_callback); // register mouse callback
    glfwSetKeyCallback(window, key_callback);         // register key callback

    // hide and lock the mouse cursor to the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // load all OpenGL function pointers
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // enable depth testing to ensure correct pixel rendering order in 3D space (depth buffer prevents incorrect overlaying and redrawing of objects)
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);                                // enable blending with the scene for particle emitters
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // use the opacity value of the particle texture to blend it correctly, ensuring smooth transparency on the edges

    // terrain reflections
    // ___________________

    // setup texture, dynamically rendered using the scene from a mirrored camera view
    unsigned int reflectionTexture;
    glGenTextures(1, &reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // setup framebuffer and renderbuffer
    // (framebuffer stores the final rendered reflection, while renderbuffer stores depth information (and optionally stencil, for more advanced effects))
    unsigned int reflectionFBO, reflectionRBO;
    glGenFramebuffers(1, &reflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0); // attach reflection texture as the storage for colors
    glGenRenderbuffers(1, &reflectionRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectionRBO); // attach renderbuffer as the storage for depth and stencil info
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // terrain shadows
    // _______________

    Shader shadowShader("shaders/shadow.vs", "shaders/shadow.fs");

    shadowShader.use();
    shadowShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, terrainOffset, 0.0f)));
    shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // setup texture, precomputed once as a shadow depth map
    unsigned int depthMapTexture;
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // setup framebuffer
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0); // attach shadow texture as the storage for depth map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // initialize entities
    // ___________________

    Skybox skybox(ourCamera);
    Water water(ourCamera, skybox.texture, reflectionTexture, depthMapTexture);
    Terrain terrain(ourCamera, skybox.texture, depthMapTexture);
    Fog fogEmitter(ourCamera, waterLevel);
    Rain rainEmitter(ourCamera, waterLevel);
    Light lightSource(ourCamera);

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
        glm::mat4 projection = glm::perspective(glm::radians(ourCamera.Zoom), (float)currentScreenWidth / (float)currentScreenHeight, 0.1f, 100.0f);

        // render reflections
        glm::vec3 reflectedPosition(ourCamera.Position.x, 2 * waterLevel - ourCamera.Position.y, ourCamera.Position.z); // reflected camera position in world space
        glm::vec3 reflectedFront(ourCamera.Front.x, -ourCamera.Front.y, ourCamera.Front.z);                             // inverted view direction for reflection
        glm::mat4 reflected_view = glm::lookAt(reflectedPosition, reflectedPosition + reflectedFront, WORLDUP);

        glViewport(0, 0, DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT); // temporarily rescale the scene to default size (a fix to render reflections correctly in fullscreen mode)
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrain.shader.use();
        terrain.shader.setMat4("view", reflected_view);
        terrain.shader.setMat4("projection", projection);

        glBindVertexArray(terrain.VAO);
        glDrawArrays(GL_TRIANGLES, 0, terrain.vertices.size() / 5); // render terrain from the reflected camera perspective
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, currentScreenWidth, currentScreenHeight);

        // render shadows
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); // temporarily rescale the scene to capture shadows in higher resolution
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowShader.use();
        glBindVertexArray(terrain.VAO);
        glDrawArrays(GL_TRIANGLES, 0, terrain.vertices.size() / 5); // render terrain from the light's perspective, though drawing shadows
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, currentScreenWidth, currentScreenHeight);

        // render main scene
        skybox.draw(view, projection, showWeather);
        water.draw(view, projection, currentFrame, deltaTime, reflected_view, showWeather, showLighting);
        terrain.draw(view, projection, showLighting);

        // render weather effects
        if (showWeather)
        {
            fogEmitter.draw(deltaTime, view, projection);
            rainEmitter.draw(deltaTime, view, projection);
        }

        // render light cube
        lightSource.draw(view, projection);

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

// whenever the mouse moves, this callback function executes
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

// whenever a key is pressed, this callback function executes and only 1 time, preventing continuous toggling when a key is held down (which would occur in processInput)
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_EQUAL:
            waveAmp += 0.1f;
            break;
        case GLFW_KEY_MINUS:
            waveAmp -= 0.1f;
            break;
        case GLFW_KEY_N:
            showWeather = !showWeather;
            break;
        case GLFW_KEY_L:
            showLighting = !showLighting;
            break;
        case GLFW_KEY_M:
            showLightSource = !showLightSource;
            break;
        case GLFW_KEY_F:
        {
            isFullscreen = !isFullscreen;
            if (isFullscreen)
            {
                // switch to fullscreen mode on primary monitor
                GLFWmonitor *monitor = glfwGetPrimaryMonitor();      // main display in the system
                const GLFWvidmode *mode = glfwGetVideoMode(monitor); // video mode (info like resolution, color depth, refresh rate)
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

                currentScreenWidth = mode->width;
                currentScreenHeight = mode->height;
            }
            else
            {
                // restore to windowed mode with default position + size
                glfwSetWindowMonitor(window, NULL, DEFAULT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y, DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT, 0);

                // reset mouse to avoid camera jump
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                lastX = xpos;
                lastY = ypos;
                firstMouse = true;

                currentScreenWidth = DEFAULT_SCR_WIDTH;
                currentScreenHeight = DEFAULT_SCR_HEIGHT;
            }
        }
        break;
        }
    }
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