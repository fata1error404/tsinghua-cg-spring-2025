#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <sstream>

class Shader
{
public:
    unsigned int shaderProgram;

    Shader(const char *vertexPath, const char *tcsPath, const char *tesPath, const char *fragmentPath)
    {
        std::string vertexCode, fragmentCode, tcsCode, tesCode;
        std::ifstream vShaderFile, fShaderFile, tcsShaderFile, tesShaderFile;
        std::stringstream vShaderStream, fShaderStream, tcsShaderStream, tesShaderStream;

        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        tcsShaderFile.open(tcsPath);
        tesShaderFile.open(tesPath);

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        tcsShaderStream << tcsShaderFile.rdbuf();
        tesShaderStream << tesShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();
        tcsShaderFile.close();
        tesShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        tcsCode = tcsShaderStream.str();
        tesCode = tesShaderStream.str();

        const char *vertexShaderSource = vertexCode.c_str();
        const char *fragmentShaderSource = fragmentCode.c_str();
        const char *tcsShaderSource = tcsCode.c_str();
        const char *tesShaderSource = tesCode.c_str();

        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int tcsShader;
        tcsShader = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcsShader, 1, &tcsShaderSource, NULL);
        glCompileShader(tcsShader);

        unsigned int tesShader;
        tesShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tesShader, 1, &tesShaderSource, NULL);
        glCompileShader(tesShader);

        unsigned int fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, tcsShader);
        glAttachShader(shaderProgram, tesShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(tcsShader);
        glDeleteShader(tesShader);
        glDeleteShader(fragmentShader);
    }

    Shader(const char *vertexPath, const char *fragmentPath)
    {
        std::string vertexCode, fragmentCode;
        std::ifstream vShaderFile, fShaderFile;
        std::stringstream vShaderStream, fShaderStream;

        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        const char *vertexShaderSource = vertexCode.c_str();
        const char *fragmentShaderSource = fragmentCode.c_str();

        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void use()
    {
        glUseProgram(shaderProgram);
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
    }

    void setMat4(const std::string &name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
};

#endif