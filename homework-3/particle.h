#ifndef PARTICLE_H
#define PARTICLE_H

struct Particle
{
    glm::vec2 Position;
    glm::vec2 Velocity;
    glm::vec4 Color;
    float Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};

class ParticleGenerator
{
public:
    float currentHue;
    float spawnTimer;
    float spawnRate;

    ParticleGenerator()
        : amount(500), emitterAngle(0.0f), rotationSpeed(5.0f),
          shader("shader.vs", "shader.fs")
    {
        particles.resize(amount);
        init();
        currentHue = 0.0f;
        spawnTimer = 0.0f;
        spawnRate = 0.08f;
    }
    void draw(float dt)
    {
        emitterAngle += rotationSpeed * dt;

        spawnTimer += dt;
        if (spawnTimer >= spawnRate)
        {
            int unused = firstUnusedParticle();
            respawnParticle(particles[unused]);
            spawnTimer = 0.0f;
        }

        std::vector<glm::vec2> offsets;
        std::vector<glm::vec4> colors;

        for (int i = 0; i < amount; ++i)
        {
            Particle &p = particles[i];

            if (p.Life > 0.0f)
            {
                p.Life -= dt;
                p.Position += p.Velocity * dt;

                float distance = glm::length(p.Position);
                float maxDistance = 3.0f;

                float alpha = glm::clamp(1.0f - distance / maxDistance, 0.0f, 1.0f);

                // Color fade: blue → red
                p.Color.a = alpha;

                // Kill particle when alpha is low (i.e. invisible)
                if (alpha < 0.01f)
                {
                    p.Life = 0.0f;
                    continue;
                }

                offsets.push_back(p.Position);
                colors.push_back(p.Color);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec2), offsets.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.setInt("particleTexture", 0);

        glBindVertexArray(VAO);
        glPointSize(40.0f);
        glDrawArraysInstanced(GL_POINTS, 0, 1, offsets.size());
        glBindVertexArray(0);
    }

private:
    std::vector<Particle> particles;
    unsigned int amount;
    Shader shader;
    unsigned int VAO, instanceVBO, colorVBO;
    float emitterAngle;
    float rotationSpeed;
    unsigned int texture;

    void init()
    {
        // orthographic projection to adjust the visible 2D world and fit the spiral on screen
        // (another approach would be to change spiral params)
        glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -4.0f, 4.0f);
        shader.use();
        shader.setMat4("projection", projection);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load("Star.bmp", &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &instanceVBO);
        glGenBuffers(1, &colorVBO);

        glBindVertexArray(VAO);

        // Positions
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
        glVertexAttribDivisor(0, 1);

        // Colors
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
        glVertexAttribDivisor(1, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void respawnParticle(Particle &particle)
    {
        glm::vec2 dir = glm::vec2(cos(emitterAngle), sin(emitterAngle));

        particle.Position = glm::vec2(0.0f);
        particle.Velocity = dir;
        particle.Life = 4.0f;

        // Smoothly cycle hue and assign fixed RGB
        glm::vec3 rgb = HSVtoRGB(currentHue, 1.0f, 1.0f);
        particle.Color = glm::vec4(rgb, 1.0f); // Start with full alpha

        currentHue += 0.01f; // Faster color transition
                             // Control smoothness of color change
        if (currentHue > 1.0f)
            currentHue -= 1.0f;
    }

    glm::vec3 HSVtoRGB(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
        float m = v - c;
        glm::vec3 rgb;

        if (h < 1.0f / 6.0f)
            rgb = glm::vec3(c, x, 0);
        else if (h < 2.0f / 6.0f)
            rgb = glm::vec3(x, c, 0);
        else if (h < 3.0f / 6.0f)
            rgb = glm::vec3(0, c, x);
        else if (h < 4.0f / 6.0f)
            rgb = glm::vec3(0, x, c);
        else if (h < 5.0f / 6.0f)
            rgb = glm::vec3(x, 0, c);
        else
            rgb = glm::vec3(c, 0, x);

        return rgb + glm::vec3(m);
    }

    int firstUnusedParticle()
    {
        for (unsigned int i = 0; i < amount; ++i)
        {
            if (particles[i].Life <= 0.0f)
                return i;
        }

        return 0;
    }
};

#endif