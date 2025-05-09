#ifndef PARTICLE_H
#define PARTICLE_H

#include <ctime>

// fog effect settings (effects applied to other entities)
const float waterFogStart = 20.0f;  // ..
const float waterFogEnd = 60.0f;    // ..
const float skyboxFogFactor = 0.6f; // blend factor toward white to simulate fog

// fog emitter settings
const float spawnRate = 0.5f;
const float particleSize = 1000.0f;
const int maxAlive = 2000;     // max number of particles allowed to be alive simultaneously
const float fogRadius = 20.0f; // radius around camera for spawning
const glm::vec4 fogColor = glm::vec4(0.8, 0.8, 0.85, 0.5);

// particle definition
struct FogParticle
{
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Life;
};

class Fog
{
public:
    Shader shader;
    Camera &camera;
    float spawnTimer;
    unsigned int VAO, VBO;
    unsigned int texture;
    std::vector<FogParticle> particles;
    float groundLevel;

    Fog(Camera &cam, float waterLevel)
        : camera(cam),
          shader("shaders/fog.vs", "shaders/fog.fs"),
          spawnTimer(0.0f),
          groundLevel(waterLevel)
    {
        particles.resize(maxAlive);

        shader.use();
        shader.setVec4("fogColor", fogColor);

        // setup buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glVertexAttribDivisor(0, 1);
        glEnableVertexAttribArray(0);

        // setup texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load("data/fog.png", &width, &height, &nrChannels, STBI_rgb_alpha);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        std::srand((unsigned)std::time(NULL));
    }

    //! Core function: spawns, kills and updates all particles.
    void draw(float dt, glm::mat4 view, glm::mat4 projection)
    {
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        std::vector<glm::vec3> positions;

        // spawn a new particle
        spawnTimer += dt;
        if (spawnTimer >= spawnRate)
        {
            int firstDead = firstUnusedParticle();
            respawnParticle(particles[firstDead]); // reuse a slot in the vector instead of continuously appending new particles to the end
            spawnTimer = 0.0f;
        }

        // update all particles
        for (int i = 0; i < maxAlive; i++)
        {
            FogParticle &p = particles[i];

            if (p.Life > 0.0f)
            {
                p.Life -= dt;
                p.Position += p.Velocity * dt;

                positions.push_back(p.Position);
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // upload positions to buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

        glDepthMask(GL_FALSE);

        // render all particles
        glPointSize(particleSize);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, positions.size());
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
    }

private:
    //! Returns the index of the first dead particle found.
    int firstUnusedParticle()
    {
        for (int i = 0; i < maxAlive; i++)
        {
            if (particles[i].Life <= 0.0f)
                return i;
        }

        return 0;
    }

    //! Updates the dead particle as a new respawned particle.
    void respawnParticle(FogParticle &p)
    {
        float randomAngle = (std::rand() / (float)RAND_MAX) * 2.0f * glm::pi<float>(); // random in range [0, 2π] (in radians)
        float randomRadius = std::sqrt(std::rand() / (float)RAND_MAX) * fogRadius;     // random in range [0, fogRadius] with bias toward center via sqrt
        float x = camera.Position.x + randomRadius * cos(randomAngle);
        float z = camera.Position.z + randomRadius * sin(randomAngle);

        p.Position = glm::vec3(x, groundLevel + 1.0f, z);

        p.Velocity = glm::vec3(
            ((std::rand() / (float)RAND_MAX) - 0.5f) * 2.0f,
            0.0f,
            ((std::rand() / (float)RAND_MAX) - 0.5f) * 2.0f); // slight random drift in horizontal direction

        p.Life = 20.0f;
    }
};

#endif