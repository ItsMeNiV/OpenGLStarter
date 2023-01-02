#ifndef __openglstarter_h_
#define __openglstarter_h_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "GLFW/glfw3.h"

inline GLFWwindow* window;

#define mainLoop while(!glfwWindowShouldClose(window))

inline int initOpenGLWithGLFW(const char* windowTitle, int windowWidth, int windowHeight)
{
    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        });
}

inline void disableCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
inline void enableCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

//Helper classes

class Shader
{
public:
    unsigned int ID;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void Use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void SetInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void SetFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void SetMat3(const std::string& name, glm::mat3& value) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void SetMat4(const std::string& name, glm::mat4& value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void SetVec3(const std::string& name, glm::vec3 value) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
    }
    // ------------------------------------------------------------------------
    void SetTexture(const std::string& name, int32_t slot)
    {
        glProgramUniform1i(ID, glGetUniformLocation(ID, name.c_str()), slot);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

class Camera
{
public:
    Camera(glm::vec3 pos, float cameraWidth, float cameraHeight)
        : cameraPos(pos), cameraUp({ 0.0f, 1.0f, 0.0f }), cameraFront({ 0.0f, 0.0f, -1.0f }), cameraRight(0.0f), worldUp(cameraUp), yaw(-90.0f), pitch(0.0f),
        projection(glm::perspective(glm::radians(45.0f), cameraWidth / cameraHeight, 0.1f, 100.0f)), view(1.0f)
    {
        UpdateCameraVectors();
    }
    ~Camera() = default;

    glm::mat4 GetViewProjection()
    {
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        return projection * view;
    }

    glm::mat4& GetView()
    {
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        return view;
    }

    glm::mat4& GetProjection()
    {
        return projection;
    }

    glm::vec3& GetPosition()
    {
        return cameraPos;
    }

    void SetPosition(glm::vec3 pos)
    {
        cameraPos = pos;
    }

    void SetYaw(float yawValue)
    {
        yaw = yawValue;
    }

    void SetPitch(float pitchValue)
    {
        pitch = pitchValue;
    }

    float GetYaw()
    {
        return yaw;
    }

    float GetPitch()
    {
        return pitch;
    }

    glm::vec3& GetCameraRight()
    {
        return cameraRight;
    }

    glm::vec3& GetCameraFront()
    {
        return cameraFront;
    }

    void UpdateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
        // also re-calculate the Right and Up vector
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }

private:
    glm::vec3 cameraPos;
    glm::vec3 cameraUp;
    glm::vec3 cameraFront;
    glm::vec3 cameraRight;
    glm::vec3 worldUp;
    // euler Angles
    float yaw;
    float pitch;

    glm::mat4 projection;
    glm::mat4 view;
};

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class CameraControllerFirstPerson
{
public:
    CameraControllerFirstPerson(Camera* camera, float moveSpeed, float mouseSens) : cameraPtr(camera), movementSpeed(moveSpeed), mouseSensitivity(mouseSens) {}

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == FORWARD)
            cameraPtr->SetPosition(cameraPtr->GetPosition() + cameraPtr->GetCameraFront() * velocity);
        if (direction == BACKWARD)
            cameraPtr->SetPosition(cameraPtr->GetPosition() - cameraPtr->GetCameraFront() * velocity);
        if (direction == LEFT)
            cameraPtr->SetPosition(cameraPtr->GetPosition() - cameraPtr->GetCameraRight() * velocity);
        if (direction == RIGHT)
            cameraPtr->SetPosition(cameraPtr->GetPosition() + cameraPtr->GetCameraRight() * velocity);
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        cameraPtr->SetYaw(cameraPtr->GetYaw() + xoffset);
        cameraPtr->SetPitch(cameraPtr->GetPitch() + yoffset);

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (cameraPtr->GetPitch() > 89.0f)
                cameraPtr->SetPitch(89.0f);
            if (cameraPtr->GetPitch() < -89.0f)
                cameraPtr->SetPitch(-89.0f);
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        cameraPtr->UpdateCameraVectors();
    }
private:
    Camera* cameraPtr;
    float movementSpeed;
    float mouseSensitivity;
};


#endif // !__openglstarter_h_
