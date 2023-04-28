#ifndef __openglstarter_h_
#define __openglstarter_h_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "GLFW/glfw3.h"

inline GLFWwindow* window;

#define mainLoop while(!glfwWindowShouldClose(window))

inline int initOpenGLWithGLFW(const char* windowTitle, int windowWidth, int windowHeight)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
}

inline void disableCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
inline void enableCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

//Helper classes

enum ShaderType
{
    VERTEX_AND_FRAGMENT, COMPUTE
};

class Shader
{
public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* sourcePath, const ShaderType type)
    {
        m_Path = std::string(sourcePath).substr(0, std::string(sourcePath).find_last_of('/') + 1);
        std::string sourceCode = readFile(sourcePath);

        if (type == ShaderType::COMPUTE)
        {
            replaceTokens(sourceCode, SourceType::COMPUTE);

            const char* cShaderCode = sourceCode.c_str();
            unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(compute, 1, &cShaderCode, NULL);
            glCompileShader(compute);
            checkCompileErrors(compute, "COMPUTE");

            m_Id = glCreateProgram();
            glAttachShader(m_Id, compute);
            glLinkProgram(m_Id);
            checkCompileErrors(m_Id, "PROGRAM");

            glDeleteShader(compute);
        }
        else if (type == ShaderType::VERTEX_AND_FRAGMENT)
        {
            std::string vertexCode = std::string(sourceCode);
            std::string fragmentCode = std::string(sourceCode);
            replaceTokens(vertexCode, SourceType::VERTEX);
            replaceTokens(fragmentCode, SourceType::FRAGMENT);

            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();
            unsigned int vertex, fragment;

            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");

            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");

            m_Id = glCreateProgram();
            glAttachShader(m_Id, vertex);
            glAttachShader(m_Id, fragment);
            glLinkProgram(m_Id);
            checkCompileErrors(m_Id, "PROGRAM");

            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void Bind()
    {
        glUseProgram(m_Id);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(m_Id, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void SetInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_Id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void SetFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(m_Id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void SetMat3(const std::string& name, glm::mat3& value) const
    {
        glUniformMatrix3fv(glGetUniformLocation(m_Id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void SetMat4(const std::string& name, glm::mat4& value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_Id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void SetVec2(const std::string& name, glm::vec2 value) const
    {
        glUniform2f(glGetUniformLocation(m_Id, name.c_str()), value.x, value.y);
    }
    // ------------------------------------------------------------------------
    void SetVec3(const std::string& name, glm::vec3 value) const
    {
        glUniform3f(glGetUniformLocation(m_Id, name.c_str()), value.x, value.y, value.z);
    }
    // ------------------------------------------------------------------------
    void SetTexture(const std::string& name, int32_t slot)
    {
        glProgramUniform1i(m_Id, glGetUniformLocation(m_Id, name.c_str()), slot);
    }
    // ------------------------------------------------------------------------
    void SetShaderStorageBlockBinding(const int32_t blockIndex, int32_t blockBinding)
    {
        glShaderStorageBlockBinding(m_Id, blockIndex, blockBinding);
    }

private:

    unsigned int m_Id;
    std::string m_Path;

    enum class SourceType : int { VERTEX = 0, FRAGMENT, COMPUTE };

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

    std::string readFile(const char* path)
    {
        std::string sourceCode;
        std::ifstream sourceFile;
        sourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            // open files
            sourceFile.open(path);
            std::stringstream sourceStream;
            // read file's buffer contents into streams
            sourceStream << sourceFile.rdbuf();
            // close file handlers
            sourceFile.close();
            // convert stream into string
            sourceCode = sourceStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << "FILEPATH: " << path << std::endl;
        }

        return sourceCode;
    }

    void replaceTokens(std::string& sourceCode, SourceType sourceType)
    {
        const char* typeString;
        switch (sourceType)
        {
        case SourceType::VERTEX:
            typeString = "VERTEX";
            break;
        case SourceType::FRAGMENT:
            typeString = "FRAGMENT";
            break;
        case SourceType::COMPUTE:
            typeString = "COMPUTE";
            break;
        }
        std::regex versionRegex("^\\#version \\d* [a-zA-Z]*\\\n");
        sourceCode = std::regex_replace(sourceCode, versionRegex, std::string("$&#define ") + typeString + "\n");

        //Replace includes
        std::smatch m;
        std::regex includeRegex("\\#include \"([^ ]*)\"");
        while (std::regex_search(sourceCode, m, includeRegex))
        {
            std::regex includeReplaceRegex("\\#include \"" + m[1].str() + "\"");
            std::string includeFile = readFile(std::string(m_Path + m[1].str()).c_str());
            sourceCode = std::regex_replace(sourceCode, includeReplaceRegex, includeFile);
        }
    }
};

struct Plane
{
    glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
    float     distance = 0.f;        // Distance with origin

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {}

    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

class Camera
{
public:
    Camera(glm::vec3 pos, float cameraWidth, float cameraHeight)
        : cameraPos(pos), cameraUp({ 0.0f, 1.0f, 0.0f }), cameraFront({ 0.0f, 0.0f, -1.0f }), cameraRight(0.0f), worldUp(cameraUp), yaw(-90.0f), pitch(0.0f),
        projection(glm::perspective(glm::radians(45.0f), cameraWidth / cameraHeight, 0.1f, 1000.0f)), view(1.0f), cameraWidth(cameraWidth), cameraHeight(cameraHeight)
    {
        UpdateCameraVectors();
    }
    ~Camera() = default;

    void UpdateWindowSize(int width, int height)
    {
        cameraWidth = width;
        cameraHeight = height;
        projection = glm::perspective(glm::radians(45.0f), cameraWidth / cameraHeight, 0.1f, 1000.0f);
    }

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

    glm::vec3& GetCameraUp()
    {
        return cameraUp;
    }

    float GetCameraWidth()
    {
        return cameraWidth;
    }

    float GetCameraHeight()
    {
        return cameraHeight;
    }

    const Frustum& GetCameraFrustum()
    {
        return cameraFrustum;
    }

    void LookAt(glm::vec3 focus)
    {
        view = glm::lookAt(cameraPos, focus, cameraUp);
        cameraFront = glm::normalize(focus - cameraPos);
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }

    void SetCameraView(glm::vec3 position, glm::vec3 lookAt, glm::vec3 up)
    {
        cameraPos = position;
        cameraFront = glm::normalize(lookAt - cameraPos);
        cameraUp = up;
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        view = glm::lookAt(cameraPos, cameraFront, cameraUp);
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

        updateFrustum();
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

    float cameraWidth;
    float cameraHeight;

    glm::mat4 projection;
    glm::mat4 view;

    Frustum cameraFrustum;

    void updateFrustum()
    {
        float aspect = cameraWidth / cameraHeight;
        float zNear = 0.1f;
        float zFar = 1000.0f;

        const float halfVSide = zFar * tanf(glm::radians(45.0f) * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * cameraFront;

        cameraFrustum.nearFace = { cameraPos + zNear * cameraFront, cameraFront };
        cameraFrustum.farFace = { cameraPos + frontMultFar, -cameraFront };
        cameraFrustum.rightFace = { cameraPos, glm::cross(frontMultFar - cameraRight * halfHSide, cameraUp) };
        cameraFrustum.leftFace = { cameraPos, glm::cross(cameraUp, frontMultFar + cameraRight * halfHSide) };
        cameraFrustum.topFace = { cameraPos, glm::cross(cameraRight, frontMultFar - cameraUp * halfVSide) };
        cameraFrustum.bottomFace = { cameraPos, glm::cross(frontMultFar + cameraUp * halfVSide, cameraRight) };
    }
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

    void SetSpeed(float moveSpeed) { movementSpeed = moveSpeed; }
private:
    Camera* cameraPtr;
    float movementSpeed;
    float mouseSensitivity;
};


#endif // !__openglstarter_h_
