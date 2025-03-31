#ifndef MESH_H
#define MESH_H

enum RenderMode
{
    WIREFRAME,
    VERTEX,
    FACE,
    FACE_EDGE
};

class Mesh
{
public:
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    Mesh(std::vector<glm::vec3> vertices, std::vector<unsigned int> indices)
    {
        this->vertices = vertices;
        this->indices = indices;
        setupMesh();
    }

    void Draw(Shader &shader, RenderMode mode)
    {
        glBindVertexArray(VAO);

        if (mode == WIREFRAME)
        {
            shader.setInt("renderMode", 1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else if (mode == VERTEX)
        {
            shader.setInt("renderMode", 2);
            glPointSize(3.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else if (mode == FACE)
        {
            shader.setInt("renderMode", 3);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }
        else if (mode == FACE_EDGE)
        {
            shader.setInt("renderMode", 4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

            shader.setInt("renderMode", 2);
            glPolygonOffset(-1.0f, -1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glLineWidth(2.0f);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_POLYGON_OFFSET_LINE);
            glLineWidth(1.0f);
        }
    }

private:
    unsigned int VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glEnableVertexAttribArray(0);
    }
};

#endif
