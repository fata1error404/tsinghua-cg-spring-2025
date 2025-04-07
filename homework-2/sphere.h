#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "font.h"
#include <unordered_map>

class Sphere
{
public:
    unsigned int textureID;
    std::unordered_map<std::string, unsigned int> textureCache;

    Sphere(unsigned int sectorCount = 36, unsigned int stackCount = 18)
        : VAO(0), VBO(0), EBO(0), initialized(false),
          sectorCount(sectorCount), stackCount(stackCount) {}

    ~Sphere()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void drawSphere(float radius, glm::vec3 orbit, const std::string &texturePath, Shader shaderMain, Shader shaderFont, std::string name, FontRenderer font, Camera camera, float rotationSpeed)
    {
        shaderMain.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, orbit);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f) * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        shaderMain.setMat4("model", model);

        if (textureCache.find(texturePath) == textureCache.end())
        {
            textureID = setupTexture(texturePath);
            if (textureID == 0)
            {
                std::cerr << "Failed to load texture: " << texturePath << std::endl;
                return;
            }
            textureCache[texturePath] = textureID;
        }
        else
            textureID = textureCache[texturePath];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        shaderMain.setInt("ourTexture", 0);

        if (!initialized || lastRadius != radius)
        {
            buildSphere(radius);
            setupBuffers();
            lastRadius = radius;
            initialized = true;
        }

        glBindVertexArray(VAO);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        // Now draw the planet's name
        shaderFont.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.f / 600.f, 0.1f, 100.0f);
        shaderFont.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shaderFont.setMat4("view", view);

        glm::vec3 worldPos = orbit;
        glm::vec4 clipSpace = projection * view * glm::vec4(worldPos, 1.0f);
        glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;

        float screenX = (ndc.x + 1.0f) * 800.f / 2.0f - 20.f;
        float screenY = (1.0f - ndc.y) * 600.f / 2.0f;
        screenY += 30.0f;

        glDisable(GL_DEPTH_TEST);
        font.Render(shaderFont.shaderProgram, name, screenX, screenY, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
    }

    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int sectorCount, stackCount;
    float lastRadius = -1.0f;
    bool initialized;

    unsigned int setupTexture(const std::string &texturePath)
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);

        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        return texture;
    }

    void buildSphere(float radius)
    {
        vertices.clear();
        indices.clear();

        const float PI = 3.14159265359f;
        float x, y, z, xy;
        float s, t;
        float sectorStep = 2 * PI / sectorCount;
        float stackStep = PI / stackCount;
        float sectorAngle, stackAngle;

        for (unsigned int i = 0; i <= stackCount; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (unsigned int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                s = (float)j / sectorCount;
                t = (float)i / stackCount;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        unsigned int k1, k2;
        for (unsigned int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);
            k2 = k1 + sectorCount + 1;

            for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
    }

    void setupBuffers()
    {
        if (VAO == 0)
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
        }

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};

#endif
