#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>


GLuint programID;
float x_delta = 0.1f;
int x_press_num = 0;


void get_OpenGL_info() {
    // OpenGL information
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void sendDataToOpenGL() {
    const GLfloat triangle[] =
    {
        -0.5f, -0.5f, +0.0f,  // left
        +1.0f, +0.0f, +0.0f,  // color
        
        +0.5f, -0.5f, +0.0f,  // right
        +1.0f, +0.0f, +0.0f,
        
        +0.0f, +0.5f, +0.0f,  // top
        +1.0f, +0.0f, +0.0f,
    };

    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);  //first VAO

    GLuint vboID;
    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

    // vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    // vertex color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));
	
	//// with indexing (uncomment to use)
    //GLuint indices[] = { 0, 1, 2 };
    //// index buffer
    //GLuint indexBufferID;
    //glGenBuffers(1, &indexBufferID);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

bool checkStatus(
                 GLuint objectID,
                 PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
                 PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
                 GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* buffer = new GLchar[infoLogLength];
        
        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
        std::cout << buffer << std::endl;
        
        delete[] buffer;
        return false;
    }
    return true;
}

bool checkShaderStatus(GLuint shaderID) {
    return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
    return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
    std::ifstream meInput(fileName);
    if (!meInput.good()) {
        std::cout << "File failed to load ... " << fileName << std::endl;
        exit(1);
    }
    return std::string(
                       std::istreambuf_iterator<char>(meInput),
                       std::istreambuf_iterator<char>()
                       );
}

void installShaders() {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar* adapter[1];
    //adapter[0] = vertexShaderCode;
    std::string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);
    //adapter[0] = fragmentShaderCode;
    temp = readShaderCode("FragmentShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);
    
    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);
    
    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
        return;
    
    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    
    if (!checkProgramStatus(programID))
        return;
    glUseProgram(programID);
    
}

void initializedGL(void) {
    // run only once
    sendDataToOpenGL();
    installShaders();
}

void paintGL(void) {
    // always run
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  //specify the background color
    glClear(GL_COLOR_BUFFER_BIT);
    
    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::translate(glm::mat4(1.0f),
                                          glm::vec3(x_delta * x_press_num, 0.0f, 0.0f));;
    GLint modelTransformMatrixUniformLocation =
    glGetUniformLocation(programID, "modelTransformMatrix");
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,
                       GL_FALSE, &modelTransformMatrix[0][0]);
    
    // glBindVertexArray();

    // without indexing
    glDrawArrays(GL_TRIANGLES, 0, 6);  //render primitives from array data
    //// with indexing (uncomment to use)
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        x_press_num -= 1;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        x_press_num += 1;
    }
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    
    /* Initialize the glfw */
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    /* glfw: configure; necessary for MAC */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    /* do not allow resizing */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "Hello Triangle", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    /* Initialize the glew */
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
    initializedGL();
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        paintGL();
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
