#ifndef LIGHT_H
#define LIGHT_H

bool showLightSource = false;

const glm::vec3 lightPos = glm::vec3(-10.0f, 6.0f, -10.0f); // world-space position of the light source
const glm::vec3 lightColor(1.0f);                           // white color

// for shadow mapping
const glm::mat4 lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f); // orthographic projection: directional light emits parallel rays, no perspective distortion
const glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), WORLDUP);        // position the light at lightPos, looking at the origin (0, 0, 0)
const glm::mat4 lightSpaceMatrix = lightProj * lightView;

// for lighting model
const float terrainAmbientStrength = 0.2f;
const float terrainDiffuseStrength = 0.8f;
const float waterAmbientStrength = 0.2f;
const float waterDiffuseStrength = 0.9f;
const float waterSpecularStrength = 0.5f;

class Light
{
public:
    Shader shader;
    Camera &camera;
    unsigned int VAO, VBO;
    unsigned int texture;

    Light(Camera &cam)
        : camera(cam),
          shader("shaders/light source.vs", "shaders/light source.fs")
    {
        shader.use();
        shader.setMat4("model", glm::translate(glm::mat4(1.0f), lightPos));
        shader.setVec3("lightColor", lightColor);

        float vertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,

            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,

            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f};

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }

    void draw(glm::mat4 view, glm::mat4 projection)
    {
        if (showLightSource)
        {
            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
    }
};

#endif