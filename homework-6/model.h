#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    std::vector<Mesh> meshes;

    Model(std::string const &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate);
        processNode(scene->mRootNode, scene);
    }

    void Draw(Shader &shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    void processNode(aiNode *node, const aiScene *scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<glm::vec3> positions(mesh->mNumVertices);
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;

            positions[i] = vector;
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        getNormals(positions, indices, normals);

        for (unsigned int i = 0; i < positions.size(); i++)
            vertices.push_back({positions[i], normals[i]});

        return Mesh(vertices, indices);
    }

    //! Calculates a normal vector for each vertex.
    void getNormals(std::vector<glm::vec3> &positions, std::vector<unsigned int> &indices, std::vector<glm::vec3> &normals)
    {
        normals.assign(positions.size(), glm::vec3(0.0f));

        // loop through each face (triangle) in the mesh
        for (int i = 0; i < indices.size(); i += 3)
        {
            // indices of the vertices that make up the current face
            unsigned int i0 = indices[i + 0];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];

            // calculate normal vector by taking the cross product of the two edge vectors
            glm::vec3 e1 = positions[i1] - positions[i0];
            glm::vec3 e2 = positions[i2] - positions[i0];
            glm::vec3 fn = glm::normalize(glm::cross(e1, e2));

            // accumulate instead of assigning (allows each vertex to average the contributions of normal vectors from all adjacent faces)
            normals[i0] += fn;
            normals[i1] += fn;
            normals[i2] += fn;
        }
    }
};

#endif