/*
Type your name and student ID here
    - Name: Your Name
    - Student ID: Your ID
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>

// 添加ImGui相关头文件
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"

// 全局变量 - 对象变换控制
GLint programID;
glm::vec3 objectPosition = glm::vec3(0.0f, 0.0f, -5.0f);
glm::vec3 objectRotation = glm::vec3(0.0f, 0.0f, 0.0f);
float objectScale = 1.0f;

// 对象VAO ID
GLuint squareVAO, cubeVAO, pyramidVAO;

// 添加键盘状态跟踪变量
struct {
    bool w, s, a, d, q, e;          // 平移键
    bool up, down, left, right;     // 旋转键
    bool z, x;                      // Z轴旋转键
    bool plus, minus;               // 缩放键
} keys;

// 移动速度和平滑参数
float moveSpeed = 2.0f;         // 每秒移动单位
float rotateSpeed = 60.0f;      // 每秒旋转角度
float scaleSpeed = 0.5f;        // 每秒缩放系数
float currentVelocity = 0.0f;   // 当前速度
float smoothness = 5.0f;        // 平滑系数，越大越平滑

// 上一帧时间
float lastFrameTime = 0.0f;

// 全局变量 - 添加窗口变量
GLFWwindow* window; // 将window声明为全局变量

// 函数声明
void get_OpenGL_info() {
    // OpenGL information
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
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
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    glUseProgram(programID);
}

void sendDataToOpenGL() {
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 1. 创建2D矩形（使用索引绘制）
    float squareVertices[] = {
        // 顶点坐标            // 颜色
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // 左下
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // 右下
         0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // 右上
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f  // 左上
    };
    
    unsigned int squareIndices[] = {
        0, 1, 2,  // 第一个三角形
        0, 2, 3   // 第二个三角形
    };
    
    glGenVertexArrays(1, &squareVAO);
    glBindVertexArray(squareVAO);
    
    GLuint squareVBO, squareEBO;
    glGenBuffers(1, &squareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &squareEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);
    
    // 顶点位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 顶点颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 2. 创建3D立方体
    float cubeVertices[] = {
        // 正面
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
        
        // 背面
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f
    };
    
    unsigned int cubeIndices[] = {
        // 正面
        0, 1, 2,
        2, 3, 0,
        // 右侧
        1, 5, 6,
        6, 2, 1,
        // 背面
        7, 6, 5,
        5, 4, 7,
        // 左侧
        4, 0, 3,
        3, 7, 4,
        // 底部
        4, 5, 1,
        1, 0, 4,
        // 顶部
        3, 2, 6,
        6, 7, 3
    };
    
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    
    GLuint cubeVBO, cubeEBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &cubeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    
    // 顶点位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 顶点颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 3. 创建3D金字塔
    float pyramidVertices[] = {
        // 底部正方形
        -0.5f, -0.5f, -0.5f,   0.5f, 0.5f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.5f, 0.5f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.5f, 0.5f, 0.0f,
        // 顶点
         0.0f,  0.5f,  0.0f,   1.0f, 1.0f, 0.0f
    };
    
    // 直接绘制，不使用索引
    
    glGenVertexArrays(1, &pyramidVAO);
    glBindVertexArray(pyramidVAO);
    
    GLuint pyramidVBO;
    glGenBuffers(1, &pyramidVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    
    // 顶点位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 顶点颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void renderControlInstructions(); // 添加前置声明

void paintGL(void) {
    // 清除颜色缓冲和深度缓冲
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置透视投影矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 20.0f);
    GLint projectionLoc = glGetUniformLocation(programID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // 设置视图矩阵
    glm::mat4 view = glm::mat4(1.0f);
    GLint viewLoc = glGetUniformLocation(programID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    // 绘制2D矩形
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, objectPosition + glm::vec3(-1.5f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(objectScale));
    
    GLint modelLoc = glGetUniformLocation(programID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(squareVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // 绘制3D立方体
    model = glm::mat4(1.0f);
    model = glm::translate(model, objectPosition);
    model = glm::rotate(model, glm::radians(objectRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(objectScale));
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // 绘制3D金字塔
    model = glm::mat4(1.0f);
    model = glm::translate(model, objectPosition + glm::vec3(1.5f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(objectRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(objectScale));
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(pyramidVAO);
    // 绘制金字塔底面
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // 绘制金字塔四个侧面
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 1, 3);
    glDrawArrays(GL_TRIANGLES, 2, 3);
    glDrawArrays(GL_TRIANGLES, 3, 3);
    
    glBindVertexArray(0);
    
    // 在3D场景渲染完成后渲染控制说明
    renderControlInstructions();
}

// 修改key_callback函数来跟踪按键状态
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // ESC键关闭窗口
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // 记录按键状态
    if (key == GLFW_KEY_W)
        keys.w = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_S)
        keys.s = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_A)
        keys.a = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_D)
        keys.d = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_Q)
        keys.q = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_E)
        keys.e = (action == GLFW_PRESS || action == GLFW_REPEAT);
    
    if (key == GLFW_KEY_UP)
        keys.up = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_DOWN)
        keys.down = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_LEFT)
        keys.left = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_RIGHT)
        keys.right = (action == GLFW_PRESS || action == GLFW_REPEAT);
    
    if (key == GLFW_KEY_Z)
        keys.z = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_X)
        keys.x = (action == GLFW_PRESS || action == GLFW_REPEAT);
    
    if (key == GLFW_KEY_EQUAL)
        keys.plus = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (key == GLFW_KEY_MINUS)
        keys.minus = (action == GLFW_PRESS || action == GLFW_REPEAT);
}

// 添加处理输入的函数
void processInput(float deltaTime) {
    // 平移控制
    if (keys.w)
        objectPosition.z -= moveSpeed * deltaTime;
    if (keys.s)
        objectPosition.z += moveSpeed * deltaTime;
    if (keys.a)
        objectPosition.x -= moveSpeed * deltaTime;
    if (keys.d)
        objectPosition.x += moveSpeed * deltaTime;
    if (keys.q)
        objectPosition.y += moveSpeed * deltaTime;
    if (keys.e)
        objectPosition.y -= moveSpeed * deltaTime;
    
    // 旋转控制
    if (keys.up)
        objectRotation.x += rotateSpeed * deltaTime;
    if (keys.down)
        objectRotation.x -= rotateSpeed * deltaTime;
    if (keys.left)
        objectRotation.y -= rotateSpeed * deltaTime;
    if (keys.right)
        objectRotation.y += rotateSpeed * deltaTime;
    if (keys.z)
        objectRotation.z += rotateSpeed * deltaTime;
    if (keys.x)
        objectRotation.z -= rotateSpeed * deltaTime;
    
    // 缩放控制 - 平滑缓冲效果
    float targetVelocity = 0.0f;
    if (keys.plus)
        targetVelocity = scaleSpeed;
    else if (keys.minus)
        targetVelocity = -scaleSpeed;
    
    // 平滑插值计算当前速度
    currentVelocity += (targetVelocity - currentVelocity) * smoothness * deltaTime;
    
    // 应用缩放
    objectScale += currentVelocity * deltaTime;
    objectScale = glm::clamp(objectScale, 0.1f, 10.0f); // 限制缩放范围
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 在initializedGL函数中添加ImGui初始化
void initializedGL(void) {
    // run only once
    sendDataToOpenGL();
    installShaders();
    
    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); // 注释掉未使用的变量或使用(void)io;消除警告
    
    // 设置ImGui样式
    ImGui::StyleColorsDark();
    
    // 设置平台/渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// 添加控制说明渲染函数

void renderControlInstructions() {
    // 开始新的ImGui帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // 创建一个窗口显示控制说明
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Control Instructions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::Text("Movement Controls:");
    ImGui::BulletText("W/S - Forward/Backward");
    ImGui::BulletText("A/D - Left/Right");
    ImGui::BulletText("Q/E - Up/Down");
    
    ImGui::Separator();
    
    ImGui::Text("Rotation Controls:");
    ImGui::BulletText("Arrow Keys - Rotate around X/Y axis");
    ImGui::BulletText("Z/X - Rotate around Z axis");
    
    ImGui::Separator();
    
    ImGui::Text("Scale Controls:");
    ImGui::BulletText("+/- - Scale up/down");
    
    ImGui::End();
    
    // 渲染ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main(int argc, char* argv[]) {
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
    window = glfwCreateWindow(800, 600, "Assignment 1", NULL, NULL);
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

    // 初始化按键状态
    memset(&keys, 0, sizeof(keys));
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* 计算时间差 */
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        /* 处理输入 */
        processInput(deltaTime);
        
        /* Render here */
        paintGL();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}