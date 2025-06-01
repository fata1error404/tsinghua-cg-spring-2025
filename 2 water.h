#ifndef WATER_H
#define WATER_H

const float waterLevel = -1.0f;            // world‐space y-axis position of the water surface
const float waterHorizontalScale = 200.0f; // scaling factor for the x and z axes

const int GRID = 100;                // water surface grid size
const float worldStep = 2.0f / GRID; // distance between neighboring grid points in world space (dx = dz)
const float waterSpeed = 0.1f;       // water texture movement speed

float waveAmp = 0.0f;
const float waveFreq = 2.0f * glm::pi<float>() / 20.0f;
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
        //! Lambda function to compute vertex position in a 2-directional Gerstner wave.
        auto computePosition = [&](float x, float z)
        {
            // start from original static position
            glm::vec3 pos{x, 0.0f, z};

            // x-direction wave (Gerstner wave formula)
            float xPhase = waveFreq * (x * waterHorizontalScale) - waveSpeed * time;
            pos.x += (waveAmp / waterHorizontalScale) * cos(xPhase); // horizontal displacement
            pos.y += waveAmp * sin(xPhase);                          // vertical displacement

            // z-direction wave
            float zPhase = waveFreq * (z * waterHorizontalScale) - waveSpeed * time;
            pos.z += (waveAmp / waterHorizontalScale) * cos(zPhase);
            pos.y += waveAmp * sin(zPhase);

            return pos;
        };

        //! Lambda function to compute vertex normal.
        auto computeNormal = [&](float x, float z)
        {
            // main neighbor vertices
            glm::vec3 v10 = computePosition(x, z - worldStep);
            glm::vec3 v12 = computePosition(x, z + worldStep);
            glm::vec3 v21 = computePosition(x - worldStep, z);
            glm::vec3 v01 = computePosition(x + worldStep, z);

            // central difference derivatives (∂y/∂x, ∂y/∂z: f'(x) ≈ (f(x + h) - f(x - h)) / (2h))
            glm::vec3 dvdx = (v01 - v21) / (2.0f * worldStep);
            glm::vec3 dvdz = (v12 - v10) / (2.0f * worldStep);

            return glm::normalize(glm::cross(dvdz, dvdx));
        };

        //! Lambda function to pack one vertex.
        auto pack = [&](int idx, glm::vec3 pos, glm::vec3 n, float u, float v)
        {
            vertices[idx + 0] = pos.x;
            vertices[idx + 1] = pos.y;
            vertices[idx + 2] = pos.z;
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

                glm::vec3 v00 = computePosition(x0, z0);
                glm::vec3 v10 = computePosition(x1, z0);
                glm::vec3 v11 = computePosition(x1, z1);
                glm::vec3 v01 = computePosition(x0, z1);

                glm::vec3 n00 = computeNormal(x0, z0);
                glm::vec3 n10 = computeNormal(x1, z0);
                glm::vec3 n11 = computeNormal(x1, z1);
                glm::vec3 n01 = computeNormal(x0, z1);

                // base vertex offset index in the 1D array
                int base = (i * GRID + j) * 6 * 8;

                // triangle 1: (00, 10, 11)
                pack(base + 0 * 8, v00, n00, 0.0f, 0.0f);
                pack(base + 1 * 8, v10, n10, 1.0f, 0.0f);
                pack(base + 2 * 8, v11, n11, 1.0f, 1.0f);

                // triangle 2: (00, 11, 01)
                pack(base + 3 * 8, v00, n00, 0.0f, 0.0f);
                pack(base + 4 * 8, v11, n11, 1.0f, 1.0f);
                pack(base + 5 * 8, v01, n01, 0.0f, 1.0f);
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