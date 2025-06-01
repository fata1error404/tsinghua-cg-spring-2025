#ifndef TERRAIN_H
#define TERRAIN_H

const float terrainOffset = waterLevel - 1.4f;    // world‐space y-axis position of the terrain
const float terrainHorizontalScale = 0.05f;       // scaling factor for the x and z axes
const float terrainVerticalScale = 5.0f / 255.0f; // scaling factor for the y-axis + pixel value conversion
const float detailLevel = 50.0f;                  // frequency of the detail texture, controlling how much detail is applied to the surface

class Terrain
{
public:
    Shader shader;
    Camera &camera;
    unsigned int VAO, VBO;
    unsigned int mainTexture, detailTexture, skyboxTexture, depthMapTexture;
    std::vector<float> vertices;

    Terrain(Camera &cam, unsigned int sky, unsigned int shadow)
        : camera(cam),
          skyboxTexture(sky),
          depthMapTexture(shadow),
          shader("shaders/terrain.vs", "shaders/terrain.fs")
    {
        shader.use();
        shader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, terrainOffset, 0.0f)));
        shader.setFloat("clipPlane", terrainOffset + 0.9f);
        shader.setFloat("detailLevel", detailLevel);
        shader.setInt("mainTexture", 0);
        shader.setInt("detailTexture", 1);
        shader.setInt("shadowMap", 2);
        shader.setInt("skyboxReflectionTexture", 3);
        shader.setVec3("skyboxScaleRatio", skyboxScaleRatio);

        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setFloat("ambientStrength", terrainAmbientStrength);
        shader.setFloat("diffuseStrength", terrainDiffuseStrength);

        int x_size, z_size, width, height, nrChannels;
        unsigned char *data = stbi_load("data/heightmap.bmp", &x_size, &z_size, &nrChannels, STBI_grey);

        //! Lambda function to pack one vertex.
        auto pack = [&](float x, float y, float z, float u, float v)
        {
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        };

        // terrain mesh generation from the height map (structured as triangles, which is optimal for rendering terrain surface: each quad → 2 triangles → 6 vertices)
        // simplified formula without scaling: vertex[i, j] = (x, y, z) = (i, heightmap[i, j], j)
        for (int i = 0; i < x_size - 1; ++i)
        {
            for (int j = 0; j < z_size - 1; ++j)
            {
                float y00 = data[i * z_size + j] * terrainVerticalScale;
                float y10 = data[i * z_size + (j + 1)] * terrainVerticalScale;
                float y11 = data[(i + 1) * z_size + (j + 1)] * terrainVerticalScale;
                float y01 = data[(i + 1) * z_size + j] * terrainVerticalScale;

                float u0 = j / (float)(z_size - 1);
                float v0 = i / (float)(x_size - 1);
                float u1 = (j + 1) / (float)(z_size - 1);
                float v1 = (i + 1) / (float)(x_size - 1);

                // triangle 1: (00, 10, 11)
                pack(j * terrainHorizontalScale, y00, i * terrainHorizontalScale, u0, v0);
                pack((j + 1) * terrainHorizontalScale, y10, i * terrainHorizontalScale, u1, v0);
                pack((j + 1) * terrainHorizontalScale, y11, (i + 1) * terrainHorizontalScale, u1, v1);

                // triangle 2: (00, 11, 01)
                pack(j * terrainHorizontalScale, y00, i * terrainHorizontalScale, u0, v0);
                pack((j + 1) * terrainHorizontalScale, y11, (i + 1) * terrainHorizontalScale, u1, v1);
                pack(j * terrainHorizontalScale, y01, (i + 1) * terrainHorizontalScale, u0, v1);
            }
        }

        stbi_image_free(data);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glGenTextures(1, &mainTexture);
        glBindTexture(GL_TEXTURE_2D, mainTexture);
        data = stbi_load("data/terrain.bmp", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glGenTextures(1, &detailTexture);
        glBindTexture(GL_TEXTURE_2D, detailTexture);
        data = stbi_load("data/detail.bmp", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    void draw(glm::mat4 view, glm::mat4 projection, bool lighting)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setBool("lighting", lighting);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mainTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, detailTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 5);
        glBindVertexArray(0);
    }
};

#endif