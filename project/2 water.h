#ifndef WATER_H
#define WATER_H

const float waterLevel = -1.0f;            // world‐space y-axis position of the water surface
const float waterHorizontalScale = 200.0f; // scaling factor for the x and z axes

const int GRID = 100;                // water surface grid size
const float worldStep = 2.0f / GRID; // distance between neighboring grid points in world space (dx = dz)
const float waterSpeed = 0.1f;       // water texture movement speed

float waveAmp = 0.0f;
const float waveFreq = 100.0f;
const float waveSpeed = 1.0f;

const float terrainReflectionStrength = 0.9f;
const float skyboxReflectionStrength = 0.3f;

class Water
{
public:
    Shader shader;
    Camera &camera;
    unsigned int VAO, VBO;
    unsigned int texture, skyboxTexture, reflectionTexture, depthMapTexture;
    std::vector<float> vertices;
    float waterOffset;

    Water(Camera &cam, unsigned int sky, unsigned int reflection, unsigned int shadow)
        : camera(cam),
          skyboxTexture(sky),
          reflectionTexture(reflection),
          depthMapTexture(shadow),
          shader("shaders/water.vs", "shaders/water.fs"),
          waterOffset(0.0f)
    {
        vertices.resize(GRID * GRID * 6 * 8);

        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(waterHorizontalScale, 1.0f, waterHorizontalScale));
        shader.setMat4("model", model);
        shader.setInt("waterTexture", 0);
        shader.setInt("shadowMap", 1);
        shader.setInt("terrainReflectionTexture", 2);
        shader.setInt("skyboxReflectionTexture", 3);
        shader.setFloat("terrainReflectionStrength", terrainReflectionStrength);
        shader.setFloat("skyboxReflectionStrength", skyboxReflectionStrength);
        shader.setVec3("skyboxScaleRatio", skyboxScaleRatio);

        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setFloat("ambientStrength", waterAmbientStrength);
        shader.setFloat("diffuseStrength", waterDiffuseStrength);
        shader.setFloat("specularStrength", waterSpecularStrength);

        shader.setVec4("fogColor", fogColor);
        shader.setFloat("fogStart", waterFogStart);
        shader.setFloat("fogEnd", waterFogEnd);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load("data/water.bmp", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    void draw(glm::mat4 view, glm::mat4 projection, float time, float dt, glm::mat4 reflected_view, bool weather, bool lighting)
    {
        //! Lambda function to compute the wave height.
        auto computeHeight = [&](float x, float z)
        {
            // Gerstner wave formula: simulate wave motion in 2 directions
            return waveAmp * (sin(waveFreq * x + waveSpeed * time) + cos(waveFreq * z + waveSpeed * time));
        };

        //! Lambda function to compute the normal at the grid point (i, j).
        auto computeNormal = [&](int i, int j)
        {
            // neighbor heights of a grid point (i, j)
            float y10 = computeHeight(i, j - 1);
            float y12 = computeHeight(i, j + 1);
            float y21 = computeHeight(i - 1, j);
            float y01 = computeHeight(i + 1, j);

            // central difference derivatives (∂y/∂x, ∂y/∂z: f'(x) ≈ (f(x + h) - f(x - h)) / (2h))
            float dydx = (y12 - y10) / (2 * worldStep);
            float dydz = (y01 - y21) * (2 * worldStep);

            return glm::normalize(glm::vec3(-dydx, 1.0f, -dydz));
        };

        //! Lambda function to pack one vertex.
        auto pack = [&](int idx, float x, float y, float z, glm::vec3 n, float u, float v)
        {
            vertices[idx + 0] = x;
            vertices[idx + 1] = y;
            vertices[idx + 2] = z;
            vertices[idx + 3] = n.x;
            vertices[idx + 4] = n.y;
            vertices[idx + 5] = n.z;
            vertices[idx + 6] = u;
            vertices[idx + 7] = v;
        };

        // 3D waves mesh generations
        for (int i = 0; i < GRID; ++i)
        {
            // convert indices to range from grid range [0, GRID - 1] to normalized coordinate range [-1, 1]
            float z0 = -1.0f + i * worldStep;
            float z1 = z0 + worldStep;

            for (int j = 0; j < GRID; ++j)
            {
                float x0 = -1.0f + j * worldStep;
                float x1 = x0 + worldStep;

                float y00 = computeHeight(x0, z0);
                float y10 = computeHeight(x1, z0);
                float y11 = computeHeight(x1, z1);
                float y01 = computeHeight(x0, z1);

                glm::vec3 n00 = computeNormal(i, j);
                glm::vec3 n10 = computeNormal(i, j + 1);
                glm::vec3 n11 = computeNormal(i + 1, j + 1);
                glm::vec3 n01 = computeNormal(i + 1, j);

                // base vertex offset index in the 1D array
                int base = (i * GRID + j) * 6 * 8;

                // triangle 1: (00, 10, 11)
                pack(base + 0 * 8, x0, y00, z0, n00, 0.0f, 0.0f);
                pack(base + 1 * 8, x1, y10, z0, n10, 1.0f, 0.0f);
                pack(base + 2 * 8, x1, y11, z1, n11, 1.0f, 1.0f);

                // triangle 2: (00, 11, 01)
                pack(base + 3 * 8, x0, y00, z0, n00, 0.0f, 0.0f);
                pack(base + 4 * 8, x1, y11, z1, n11, 1.0f, 1.0f);
                pack(base + 5 * 8, x0, y01, z1, n01, 0.0f, 1.0f);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("reflected_view", reflected_view);
        shader.setBool("lighting", lighting);
        shader.setBool("weather", weather);
        shader.setVec3("cameraPos", camera.Position);

        waterOffset += waterSpeed * dt;
        shader.setFloat("offset", waterOffset);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, reflectionTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);
        glBindVertexArray(0);
    }
};

#endif