#ifndef PARTICLE_H
#define PARTICLE_H

#include <ctime>

struct Particle
{
    glm::vec2 Position;
    glm::vec2 Velocity;
    float Life;
    float Size;
};

const int maxAlive = 2000;
const float maxSpawnRate = 0.05f;
const float spawnIncreaseRate = 0.0005f;
const float fallSpeed = -0.2f;

class ParticleEmitter
{
public:
    std::vector<Particle> particles;
    Shader shader;
    float spawnTimer;
    float spawnRate;
    unsigned int VAO, posVBO, sizeVBO;
    unsigned int texture;

    ParticleEmitter()
        : spawnTimer(0.0f),
          spawnRate(1.0f),
          shader("particle shader.vs", "particle shader.fs")
    {
        particles.resize(maxAlive);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load("snowflake.png", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        shader.setInt("particleTexture", 0);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &posVBO);
        glGenBuffers(1, &sizeVBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
        glVertexAttribDivisor(0, 1);
        glEnableVertexAttribArray(0);

        // each particle has its own size, which will be stored in the VBO used as an attribute for instanced drawing
        glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
        glVertexAttribDivisor(1, 1);
        glEnableVertexAttribArray(1);

        glEnable(GL_PROGRAM_POINT_SIZE); // allow vertex shader to control point size

        std::srand(std::time(NULL)); // seed the random number generator to produce different results on each program launch
    }

    void draw(float dt)
    {
        std::vector<glm::vec2> positions;
        std::vector<float> sizes;

        // spawn rate starts at 1 particle / sec and decays by spawnIncreaseRate until it reaches maxSpawnRate
        spawnRate = std::max(maxSpawnRate, spawnRate - spawnIncreaseRate);
        spawnTimer += dt;
        if (spawnTimer >= spawnRate)
        {
            int firstDead = firstUnusedParticle();
            respawnParticle(particles[firstDead]);
            spawnTimer = 0.0f;
        }

        for (int i = 0; i < maxAlive; i++)
        {
            Particle &p = particles[i];

            if (p.Life > 0.0f)
            {
                p.Position += p.Velocity * dt;

                // kill particle when it goes below the screen
                if (p.Position.y < -1.1f)
                {
                    p.Life = 0.0f;
                    continue;
                }

                positions.push_back(p.Position);
                sizes.push_back(p.Size);
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizes.size() * sizeof(float), sizes.data(), GL_DYNAMIC_DRAW);

        shader.use();
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, positions.size());
        glBindVertexArray(0);
    }

private:
    int firstUnusedParticle()
    {
        for (unsigned int i = 0; i < maxAlive; i++)
        {
            if (particles[i].Life <= 0.0f)
                return i;
        }

        return 0;
    }

    void respawnParticle(Particle &particle)
    {
        float randomX = -1.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (2.0f)));             // random in range [-1, 1]
        float randomSize = 10.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (50.0f - 10.0f))); // random in range [10, 50]

        particle.Position = glm::vec2(randomX, 1.0f);
        particle.Velocity = glm::vec2(0.0f, fallSpeed);
        particle.Life = 1.0f;
        particle.Size = randomSize;
    }
};

#endif