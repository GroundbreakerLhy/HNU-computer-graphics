/*
Type your name and student ID here
    - Name: Your Name
    - Student ID: Your ID
*/

#include "../../../Dependencies/glew/glew.h"
#include "../../../Dependencies/GLFW/glfw3.h"
#include "../../../Dependencies/glm/glm.hpp" 
#include "../../../Dependencies/glm/gtc/matrix_transform.hpp"
#include "../../../Dependencies/glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>

#include "../../../imgui/imgui.h"
#include "../../../imgui/backends/imgui_impl_glfw.h"
#include "../../../imgui/backends/imgui_impl_opengl3.h"

// 对象变换控制
GLint programID;

// 添加选中对象状态跟踪
enum SelectedObject {
    NONE = -1,
    SQUARE = 0,
    CUBE = 1,
    PYRAMID = 2
};

// 定义每个对象的变换参数
struct ObjectTransform {
    glm::vec3 position;
    glm::vec3 rotation;
    float scale;
};

// 当前选中的对象
SelectedObject selectedObject = NONE;

// 对象变换参数
ObjectTransform objectTransforms[3] = {
    // 矩形 
    { glm::vec3(-1.5f, 0.0f, -5.0f), glm::vec3(0.0f), 1.0f },
    // 立方体
    { glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), 1.0f },
    // 金字塔
    { glm::vec3(1.5f, 0.0f, -5.0f), glm::vec3(0.0f), 1.0f }
};

// 对象VAO ID
GLuint squareVAO, cubeVAO, pyramidVAO;

// 键盘状态跟踪变量
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
float smoothness = 5.0f;        // 平滑系数

// 上一帧时间
float lastFrameTime = 0.0f;

// 窗口变量
GLFWwindow* window;

// 添加鼠标拖动状态跟踪
bool isDragging = false;
double lastMouseX, lastMouseY;

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
    std::string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);
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
        -0.5f, -0.5f, -0.5f,   0.5f, 0.5f, 0.0f,  // 0
         0.5f, -0.5f, -0.5f,   0.5f, 0.5f, 0.0f,  // 1
         0.5f, -0.5f,  0.5f,   0.5f, 0.5f, 0.0f,  // 2
        -0.5f, -0.5f,  0.5f,   0.5f, 0.5f, 0.0f,  // 3
        // 顶点
         0.0f,  0.5f,  0.0f,   1.0f, 1.0f, 0.0f   // 4
    };

    unsigned int pyramidIndices[] = {
        // 底面
        0, 1, 2,
        2, 3, 0,
        // 侧面
        0, 1, 4,  // 前侧面
        1, 2, 4,  // 右侧面 
        2, 3, 4,  // 后侧面
        3, 0, 4   // 左侧面
    };

    glGenVertexArrays(1, &pyramidVAO);
    glBindVertexArray(pyramidVAO);

    GLuint pyramidVBO, pyramidEBO;
    glGenBuffers(1, &pyramidVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    // 创建EBO并绑定
    glGenBuffers(1, &pyramidEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    // 顶点位置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 顶点颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO
    glBindVertexArray(0);
}

void renderControlInstructions();

// 射线-物体相交检测
bool checkRayObjIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                            const glm::vec3& objPos, float objScale, float& distance) {
    // 碰撞箱检测
    float radius = objScale * 0.5f;
    glm::vec3 oc = rayOrigin - objPos;
    
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return false;
    } else {
        distance = (-b - sqrt(discriminant)) / (2.0f * a);
        return (distance > 0);
    }
}

// 鼠标点击回调
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // 获取鼠标在窗口中的位置
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            // 将屏幕坐标转换为NDC坐标
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            
            float x = (2.0f * xpos) / width - 1.0f;
            float y = 1.0f - (2.0f * ypos) / height;
            
            // 创建从屏幕到世界的射线
            glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 20.0f);
            glm::mat4 view = glm::mat4(1.0f);
            
            glm::vec4 rayEye = glm::inverse(projection) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
            
            glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
            rayWorld = glm::normalize(rayWorld);
            
            glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
            
            // 检测射线与每个对象的相交
            float minDist = FLT_MAX;
            int closestObj = -1;
            
            for (int i = 0; i < 3; i++) {
                float dist;
                if (checkRayObjIntersection(rayOrigin, rayWorld, objectTransforms[i].position, 
                                            objectTransforms[i].scale, dist)) {
                    if (dist < minDist) {
                        minDist = dist;
                        closestObj = i;
                    }
                }
            }
            
            // 更新选中的对象
            selectedObject = (closestObj >= 0) ? static_cast<SelectedObject>(closestObj) : NONE;
            
            // 如果选中了对象，开始拖动
            if (selectedObject != NONE) {
                isDragging = true;
                lastMouseX = xpos;
                lastMouseY = ypos;
            }
        } else if (action == GLFW_RELEASE) {
            // 释放鼠标，停止拖动
            isDragging = false;
        }
    }
}

// 添加鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging && selectedObject != NONE) {
        // 计算鼠标移动距离
        float dx = xpos - lastMouseX;
        float dy = lastMouseY - ypos; // 反转Y轴
        
        // 计算移动敏感度
        float sensitivity = 0.01f;
        
        // 移动选中的对象
        ObjectTransform& transform = objectTransforms[selectedObject];
        
        // X和Y平面上的移动
        transform.position.x += dx * sensitivity;
        transform.position.y += dy * sensitivity;
        
        // 更新鼠标位置
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void paintGL(void) {
    
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
    
    // 绘制2D矩形，高亮选中的对象
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, objectTransforms[SQUARE].position);
        model = glm::rotate(model, glm::radians(objectTransforms[SQUARE].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[SQUARE].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[SQUARE].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(objectTransforms[SQUARE].scale));
        
        // 如果是选中的对象，提供视觉反馈
        if (selectedObject == SQUARE) {
            // 设置边界颜色或其他视觉指示
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 1);
        } else {
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 0);
        }
        
        GLint modelLoc = glGetUniformLocation(programID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(squareVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    // 绘制3D立方体
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, objectTransforms[CUBE].position);
        model = glm::rotate(model, glm::radians(objectTransforms[CUBE].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[CUBE].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[CUBE].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(objectTransforms[CUBE].scale));
        
        // 如果是选中的对象，提供视觉反馈
        if (selectedObject == CUBE) {
            // 设置边界颜色或其他视觉指示
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 1);
        } else {
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 0);
        }
        
        GLint modelLoc = glGetUniformLocation(programID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    
    // 绘制3D金字塔
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, objectTransforms[PYRAMID].position);
        model = glm::rotate(model, glm::radians(objectTransforms[PYRAMID].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[PYRAMID].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(objectTransforms[PYRAMID].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(objectTransforms[PYRAMID].scale));
        
        // 如果是选中的对象，提供视觉反馈
        if (selectedObject == PYRAMID) {
            // 设置边界颜色或其他视觉指示
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 1);
        } else {
            GLint highlightLoc = glGetUniformLocation(programID, "highlight");
            glUniform1i(highlightLoc, 0);
        }
        
        GLint modelLoc = glGetUniformLocation(programID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(pyramidVAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    }
    
    glBindVertexArray(0);
    
    // 在3D场景渲染完成后渲染控制说明
    renderControlInstructions();
}

// 修改key_callback函数来跟踪按键状态
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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

// 处理输入的函数
void processInput(float deltaTime) {
    if (selectedObject == NONE)
        return;
    
    ObjectTransform& transform = objectTransforms[selectedObject];
    
    // 平移控制
    if (keys.w)
        transform.position.z -= moveSpeed * deltaTime;
    if (keys.s)
        transform.position.z += moveSpeed * deltaTime;
    if (keys.a)
        transform.position.x -= moveSpeed * deltaTime;
    if (keys.d)
        transform.position.x += moveSpeed * deltaTime;
    if (keys.q)
        transform.position.y += moveSpeed * deltaTime;
    if (keys.e)
        transform.position.y -= moveSpeed * deltaTime;
    
    // 旋转控制
    if (keys.up)
        transform.rotation.x += rotateSpeed * deltaTime;
    if (keys.down)
        transform.rotation.x -= rotateSpeed * deltaTime;
    if (keys.left)
        transform.rotation.y -= rotateSpeed * deltaTime;
    if (keys.right)
        transform.rotation.y += rotateSpeed * deltaTime;
    if (keys.z)
        transform.rotation.z += rotateSpeed * deltaTime;
    if (keys.x)
        transform.rotation.z -= rotateSpeed * deltaTime;
    
    // 缩放控制
    float targetVelocity = 0.0f;
    if (keys.plus)
        targetVelocity = scaleSpeed;
    else if (keys.minus)
        targetVelocity = -scaleSpeed;
    
    // 计算当前速度
    currentVelocity += (targetVelocity - currentVelocity) * smoothness * deltaTime;
    
    // 应用缩放
    transform.scale += currentVelocity * deltaTime;
    transform.scale = glm::clamp(transform.scale, 0.1f, 10.0f); // 限制缩放范围
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 在initializedGL函数中添加ImGui初始化
void initializedGL(void) {
    sendDataToOpenGL();
    installShaders();
    
    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
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
    ImGui::SetNextWindowSize(ImVec2(350, 240), ImGuiCond_FirstUseEver);
    ImGui::Begin("Control Instructions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    // 选择提示
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FIRST: Click on an object to select it!");
    
    // 显示当前选择状态
    if (selectedObject == NONE) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No object selected");
    } else {
        const char* objNames[] = { "Square", "Cube", "Pyramid" };
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Selected: %s", objNames[selectedObject]);
    }
    
    ImGui::Separator();
    
    ImGui::Text("Movement Controls (after selection):");
    ImGui::BulletText("W/S - Forward/Backward");
    ImGui::BulletText("A/D - Left/Right");
    ImGui::BulletText("Q/E - Up/Down");
    ImGui::BulletText("Mouse Drag - Move in X/Y plane");
    
    ImGui::Separator();
    
    ImGui::Text("Rotation Controls (after selection):");
    ImGui::BulletText("Arrow Keys - Rotate around X/Y axis");
    ImGui::BulletText("Z/X - Rotate around Z axis");
    
    ImGui::Separator();
    
    ImGui::Text("Scale Controls (after selection):");
    ImGui::BulletText("+/- - Scale up/down");
    
    ImGui::End();
    
    // 渲染ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(1200, 900, "Assignment 1", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
    initializedGL();

    memset(&keys, 0, sizeof(keys));
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        processInput(deltaTime);
        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}