#ifndef RAIN_H
#define RAIN_H

#include <ctime>

// rain emitter settings
const int numDrops = 10000;                                                   // number of raindrops
const float spawnHeight = 10.0f;                                              // min height at which raindrops spawn
const float rainRadius = 20.0f;                                               // radius around camera for spawning
const float rainSpeed = 4.0f;                                                 // raindrop fall speed
const glm::vec3 windDirection = glm::normalize(glm::vec3(1.0f, -4.0f, 0.3f)); // raindrop fall angle (angled to the right and a bit forward)
const glm::vec4 rainColor = glm::vec4(0.5, 0.6, 0.9, 1.0);                    // raindrop color

// particle definition
struct RainParticle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
};

class Rain
{
public:
    Shader shader;
    Camera &camera;
    unsigned int VAO, VBO;
    unsigned int texture;
    std::vector<RainParticle> particles;
    float groundLevel;

    // constructor that sets up the initial emitter configuration
    Rain(Camera &cam, float waterLevel)
        : camera(cam),
          shader("shaders/rain.vs", "shaders/rain.fs"),
          groundLevel(waterLevel)
    {
        particles.resize(numDrops);

        shader.use();
        shader.setVec4("rainColor", rainColor);
        shader.setFloat("spawnHeight", spawnHeight);

        // setup buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glVertexAttribDivisor(0, 1); // needed for instance rendering: to update the attribute at location 0 once per instance
        glEnableVertexAttribArray(0);

        glEnable(GL_PROGRAM_POINT_SIZE); // allow vertex shader to control point size

        // setup texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load("data/rain.png", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        std::srand(std::time(NULL)); // seed the random number generator to produce different results on each program launch
    }

    //! Core function: spawns, kills and updates all particles.
    void draw(float dt, const glm::mat4 &view, const glm::mat4 &projection)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("cameraPos", camera.Position);

        std::vector<glm::vec3> positions;

        // update all particles
        for (int i = 0; i < numDrops; i++)
        {
            RainParticle &p = particles[i];

            p.Position += p.Velocity * dt;

            // respawn particle if reached ground
            if (p.Position.y <= groundLevel + 1.0f)
                respawnParticle(p);

            positions.push_back(p.Position);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // upload positions to buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

        glDepthMask(GL_FALSE); // disable writing to the depth buffer while rendering particles, preventing them from overlaying each other

        // render all particles
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, positions.size()); // instance rendering: draws many objects with one function call, using different attributes per instance (more efficient than a for loop)
        glBindVertexArray(0);

        glDepthMask(GL_TRUE); // re-enable for the rest of the scene
    }

private:
    //! Updates the dead particle as a new respawned particle.
    void respawnParticle(RainParticle &p)
    {
        float randomOffset = static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 10.0f)); // random in range [0, 10]
        float randomAngle = (std::rand() / (float)RAND_MAX) * 2.0f * glm::pi<float>();                 // random in range [0, 2π] (in radians)
        float randomRadius = std::sqrt(std::rand() / (float)RAND_MAX) * rainRadius;                    // random in range [0, rainRadius] with bias toward center via sqrt
        float x = camera.Position.x + randomRadius * cos(randomAngle);
        float z = camera.Position.z + randomRadius * sin(randomAngle);
        float y = spawnHeight + randomOffset;

        p.Position = glm::vec3(x, y, z);
        p.Velocity = windDirection * rainSpeed;
    }
};

#endif