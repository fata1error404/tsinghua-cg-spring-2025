#ifndef PARTICLE_H
#define PARTICLE_H

// particle definition
struct Particle
{
    glm::vec2 Position;
    glm::vec2 Velocity;
    glm::vec4 Color;
    float Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};

// particle emitter settings
const int maxAlive = 500;         // max number of particles allowed to be alive simultaneously
const float spawnRate = 0.08f;    // min time interval between spawning two consecutive particles (in seconds)
const float rotationSpeed = 5.0f; // angular velocity of the emitter (radians per second)
const float maxDistance = 3.0f;   // distance at which a particle's opacity fades to zero
const float minOpacity = 0.01f;   // opacity threshold below which a particle is considered dead

class ParticleEmitter
{
public:
    std::vector<Particle> particles;
    Shader shader;
    float spawnTimer;
    float emitterAngle;
    float hue;
    unsigned int VAO, posVBO, colorVBO;
    unsigned int texture;

    //! Constructor that sets up the initial emitter configuration.
    ParticleEmitter()
        : spawnTimer(0.0f),
          emitterAngle(0.0f),
          hue(0.0f),
          shader("shader.vs", "shader.fs")
    {
        particles.resize(maxAlive);

        // orthographic projection to adjust the visible 2D world and fit the spiral on screen
        // (another approach would be to change spiral params)
        glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -4.0f, 4.0f);
        shader.use();
        shader.setMat4("projection", projection);

        // setup texture
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

        shader.setInt("particleTexture", 0);

        // setup buffers
        // 1 VAO to store overall state; 2 VBOs separately for positions and colors (because they are stored in different variables)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &posVBO);
        glGenBuffers(1, &colorVBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
        glVertexAttribDivisor(0, 1); // needed for instance rendering: to update the attribute at location 0 once per instance
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);
        glVertexAttribDivisor(1, 1);
        glEnableVertexAttribArray(1);
    }

    //! Core function: spawns, kills and updates all particles.
    void draw(float dt)
    {
        std::vector<glm::vec2> positions;
        std::vector<glm::vec4> colors;

        emitterAngle += rotationSpeed * dt;

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
            Particle &p = particles[i];

            if (p.Life > 0.0f)
            {
                // reduce the particle's remaining life, position, and opacity in real time
                p.Life -= dt;
                p.Position += p.Velocity * dt;
                p.Color.a = glm::clamp(1.0f - glm::length(p.Position) / maxDistance, 0.0f, 1.0f); // fade out the particle's opacity based on its distance from emitter

                // kill particle when opacity is low (i.e. invisible)
                if (p.Color.a < minOpacity)
                {
                    p.Life = 0.0f;
                    continue;
                }

                positions.push_back(p.Position);
                colors.push_back(p.Color);
            }
        }

        // update buffers
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);

        // render all particles
        glPointSize(40.0f);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, positions.size()); // instance rendering: draws many objects with one function call, using different attributes per instance (more efficient than a for loop)
        glBindVertexArray(0);
    }

private:
    //! Returns the index of the first dead particle found.
    int firstUnusedParticle()
    {
        for (unsigned int i = 0; i < maxAlive; i++)
        {
            if (particles[i].Life <= 0.0f)
                return i;
        }

        return 0;
    }

    //! Updates the dead particle as a new respawned particle.
    void respawnParticle(Particle &particle)
    {
        particle.Position = glm::vec2(0.0f);                                 // no offset from the emitter's origin
        particle.Velocity = glm::vec2(cos(emitterAngle), sin(emitterAngle)); // Archimedes spiral formula: emitter angle defines velocity of a new particle
        particle.Color = glm::vec4(HSVtoRGB(hue, 1.0f, 1.0f), 1.0f);         // smooth color transition using HSV representation of RGB color space
        particle.Life = 4.0f;

        hue += 0.01f;

        if (hue > 1.0f)
            hue -= 1.0f;
    }

    //! Converts a color from HSV (Hue, Saturation, Value) to RGB (Red, Green, Blue) format, which is needed for OpenGL color rendering.
    glm::vec3 HSVtoRGB(float h, float s, float v)
    {
        float c = v * s;                                      // chroma: intensity of color (difference between max and min RGB components)
        float m = v - c;                                      // minimum RGB component (used to shift RGB to match brightness)
        float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1)); // helper value

        glm::vec3 rgb;

        // determine which section of the color wheel hue falls into (6 sectors)
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
};

#endif