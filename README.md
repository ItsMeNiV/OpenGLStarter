# OpenGLStarter
Header-Only Library which makes creating new OpenGL Projects easier

## Features
 * Easy one-line initialization of GLFW and Glad
 * Helper Classes for Shaders and Cameras to reduce boilerplate-code

## Usage
Just include the header file "OpenGLStarter.h" into your project and add the following external libraries:
 * GLFW
 * Glad
 * GLM

```c++
#include "OpenGLStarter.h"

int main()
{
    initOpenGLWithGLFW("Example window", 900, 600);

    Shader exampleShader("example-shaders/exampleshader.glsl", ShaderType::VERTEX_AND_FRAGMENT);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    unsigned int vertexBuffer, vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);
    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    mainLoop
    { 
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        exampleShader.Bind();
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
```
