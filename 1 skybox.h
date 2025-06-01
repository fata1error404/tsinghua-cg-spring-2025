#ifndef SKYBOX_H
#define SKYBOX_H

const glm::vec3 skyboxScaleRatio = glm::vec3(1.0f, 1.0f, 1.4f); // skybox ratio control parameters (z, y, z) to maintain skybox proportions

class Skybox
{
public:
    Shader shader;
    Camera &camera;
    unsigned int VAO, VBO;
    unsigned int texture;

    Skybox(Camera &cam)
        : camera(cam),
          shader("shaders/skybox.vs", "shaders/skybox.fs")
    {
        shader.use();
        shader.setFloat("fogDensity", skyboxFogFactor);

        glDepthFunc(GL_LEQUAL); // ensure the skybox fail the depth test wherever there's a different object in front of it (its depth is set to 1.0 in the vertex shader, so we need less or equal depth function)

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

        // setup buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // load and configure a cubemap texture for a skybox using six face images
        std::vector<std::string> faces{
            "data/skybox/SkyBox1.bmp",
            "data/skybox/SkyBox3.bmp",
            "data/skybox/SkyBox4.bmp",
            "data/skybox/SkyBox4.bmp",
            "data/skybox/SkyBox0.bmp",
            "data/skybox/SkyBox2.bmp",
        };

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
        }
    }

    void draw(glm::mat4 view, glm::mat4 projection, bool weather)
    {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, skyboxScaleRatio);                                               // correct skybox, so that the sun is a circle
        model = glm::translate(model, glm::vec3(0.0f, -(camera.Position.y + 1.0f) * 0.03f, 0.0f)); // hard-coded (TODO)
        shader.setMat4("model", model);
        shader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix so the skybox moves with the camera, creating the illusion of an infinitely distant environments
        shader.setMat4("projection", projection);
        shader.setBool("weather", weather);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};

#endif