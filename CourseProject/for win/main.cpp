/*
太空黄金收藏项目
Student Information
Student ID:
Student Name:
*/

#include "../../Dependencies/glew/glew.h"
#include "../../Dependencies/GLFW/glfw3.h"

#include "../../Dependencies/glm/glm.hpp"
#include "../../Dependencies/glm/gtc/matrix_transform.hpp"
#include "../../Dependencies/glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <map>
#include <deque>
#include <random>
#include <sstream>

const int SCR_WIDTH = 2400;
const int SCR_HEIGHT = 1600;

// GLFW 错误回调
static void error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

// 小行星数据结构
struct Asteroid {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 rotationSpeed;
    float scale;
    bool isGold;
    bool collected;
};

// 本地太空飞行器数据结构
struct LocalCraft {
    glm::vec3 position;
    glm::vec3 direction;
    float rotation;
    float speed;
    bool alerted;
    float orbitRadius;     // 轨道半径
    float orbitAngle;      // 当前轨道角度
    float orbitSpeed;      // 公转速度
    float orbitHeight;     // 轨道高度
    float orbitInclination; // 轨道倾斜角度
    float inclinationSpeed; // 倾斜角变化速度
};

Model loadOBJ(const char* objPath)
{
    struct V {
        unsigned int index_position, index_uv, index_normal;
        bool operator == (const V& v) const {
            return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
        }
        bool operator < (const V& v) const {
            return (index_position < v.index_position) ||
                (index_position == v.index_position && index_uv < v.index_uv) ||
                (index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
        }
    };

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::map<V, unsigned int> temp_vertices;

    Model model;
    unsigned int num_vertices = 0;

    std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

    std::ifstream file;
    file.open(objPath);

    if (file.fail()) {
        std::cerr << "Impossible to open the file! Do you use the right path?" << std::endl;
        exit(1);
    }

    while (!file.eof()) {
        char lineHeader[128];
        file >> lineHeader;

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 position;
            file >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string line;
            std::getline(file, line);
            
            // 跳过空行
            if (line.empty() || line.find_first_not_of(" \r") == std::string::npos) {
                continue;
            }
            
            // 解析面定义，支持三角形和四边形
            std::istringstream iss(line);
            std::vector<std::string> faceTokens;
            std::string token;
            
            while (iss >> token) {
                faceTokens.push_back(token);
            }
            
            if (faceTokens.size() == 3) {
                // 三角形面
                V vertices[3];
                for (int i = 0; i < 3; i++) {
                    char ch;
                    std::istringstream tokenStream(faceTokens[i]);
                    tokenStream >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
                }

                for (int i = 0; i < 3; i++) {
                    if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
                        Vertex vertex;
                        vertex.position = temp_positions[vertices[i].index_position - 1];
                        vertex.uv = temp_uvs[vertices[i].index_uv - 1];
                        vertex.normal = temp_normals[vertices[i].index_normal - 1];

                        model.vertices.push_back(vertex);
                        model.indices.push_back(num_vertices);
                        temp_vertices[vertices[i]] = num_vertices;
                        num_vertices += 1;
                    }
                    else {
                        unsigned int index = temp_vertices[vertices[i]];
                        model.indices.push_back(index);
                    }
                }
            }
            else if (faceTokens.size() == 4) {
                // 四边形面 - 分解为两个三角形
                V vertices[4];
                for (int i = 0; i < 4; i++) {
                    char ch;
                    std::istringstream tokenStream(faceTokens[i]);
                    tokenStream >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
                }

                // 第一个三角形: 0, 1, 2
                int triangleIndices1[] = {0, 1, 2};
                for (int i = 0; i < 3; i++) {
                    int idx = triangleIndices1[i];
                    if (temp_vertices.find(vertices[idx]) == temp_vertices.end()) {
                        Vertex vertex;
                        vertex.position = temp_positions[vertices[idx].index_position - 1];
                        vertex.uv = temp_uvs[vertices[idx].index_uv - 1];
                        vertex.normal = temp_normals[vertices[idx].index_normal - 1];

                        model.vertices.push_back(vertex);
                        model.indices.push_back(num_vertices);
                        temp_vertices[vertices[idx]] = num_vertices;
                        num_vertices += 1;
                    }
                    else {
                        unsigned int index = temp_vertices[vertices[idx]];
                        model.indices.push_back(index);
                    }
                }

                // 第二个三角形: 0, 2, 3
                int triangleIndices2[] = {0, 2, 3};
                for (int i = 0; i < 3; i++) {
                    int idx = triangleIndices2[i];
                    if (temp_vertices.find(vertices[idx]) == temp_vertices.end()) {
                        Vertex vertex;
                        vertex.position = temp_positions[vertices[idx].index_position - 1];
                        vertex.uv = temp_uvs[vertices[idx].index_uv - 1];
                        vertex.normal = temp_normals[vertices[idx].index_normal - 1];

                        model.vertices.push_back(vertex);
                        model.indices.push_back(num_vertices);
                        temp_vertices[vertices[idx]] = num_vertices;
                        num_vertices += 1;
                    }
                    else {
                        unsigned int index = temp_vertices[vertices[idx]];
                        model.indices.push_back(index);
                    }
                }
            }
            else if (faceTokens.size() > 0) {
                std::cerr << "Warning: Face with " << faceTokens.size() << " vertices found, skipping." << std::endl;
            }
        }
        else {
            char stupidBuffer[1024];
            file.getline(stupidBuffer, 1024);
        }
    }
    file.close();

    std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
    return model;
}

void get_OpenGL_info()
{
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

// 全局变量
Shader* shader;
Shader* skyboxShader;
Model planetModel, spacecraftModel, rockModel, craftModel;

// 模型VAO
GLuint planetVAO, planetVBO, planetEBO;
GLuint spacecraftVAO, spacecraftVBO, spacecraftEBO;
GLuint rockVAO, rockVBO, rockEBO;
GLuint craftVAO, craftVBO, craftEBO;
GLuint skyboxVAO, skyboxVBO;

// Skybox纹理
GLuint skyboxTexture;

// 纹理对象
Texture planetTexture, planetNormalMap;
Texture spacecraftTexture, spacecraftGoldTexture;
Texture rockTexture;
Texture goldTexture;
Texture craftTexture, craftAlertTexture;

// 光照参数
glm::vec3 lightPosition1 = glm::vec3(10.0f, 10.0f, 10.0f);
glm::vec3 lightPosition2 = glm::vec3(-10.0f, 5.0f, -10.0f);
float lightIntensity1 = 1.0f;
float lightIntensity2 = 0.8f;

// 场景对象
glm::vec3 planetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
float planetRotation = 0.0f;
glm::vec3 spacecraftPosition = glm::vec3(0.0f, 0.0f, 600.0f);
glm::vec3 spacecraftRotation = glm::vec3(0.0f, 0.0f, glm::radians(180.0f)); 

// 小行星环
const int NUM_ASTEROIDS = 1600;
std::vector<Asteroid> asteroids;
const int NUM_GOLD_ASTEROIDS = 50;
int collectedGold = 0;

// 本地太空飞行器
const int NUM_LOCAL_CRAFTS = 10;
std::vector<LocalCraft> localCrafts;

// 摄像机参数
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 鼠标控制参数
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool leftMousePressed = false;

// 游戏状态
bool gameCompleted = false;

bool keys[1024]; // 键盘状态数组
float yaw = -90.0f;
float pitch = 0.0f; // 垂直旋转角度
float baseSensitivity = 0.01f; // 基础灵敏度
float minSensitivity = 0.008f;  // 最小灵敏度
float maxSensitivity = 0.016f;  // 最大灵敏度

// 非线性灵敏度控制变量
float lastMouseSpeed = 0.0f;
float smoothedMouseSpeed = 0.0f;
float speedDecayFactor = 0.8f;

// 游戏状态变量
int totalGoldCollected = 0;
bool alertTriggered = false;

float currentRoll = 0.0f;
float targetRoll = 0.0f;
float rollSpeed = 0.05f;
float maxRoll = 25.0f;

std::string cheatCode = "";
const std::string GOLD_CHEAT = "uuddllrrbaba";
bool cheatActivated = false;
int lastKey = -1;


bool isCollisionFrozen = false;
float freezeTimer = 0.0f;
const float FREEZE_DURATION = 1.0f;
bool shouldResetGame = false;

void setupModel(GLuint& VAO, GLuint& VBO, GLuint& EBO, const Model& model) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), &model.indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    glBindVertexArray(0);
}

void setupSkybox() {
    float skyboxVertices[] = {
        // positions          
        -500.0f,  500.0f, -500.0f,
        -500.0f, -500.0f, -500.0f,
         500.0f, -500.0f, -500.0f,
         500.0f, -500.0f, -500.0f,
         500.0f,  500.0f, -500.0f,
        -500.0f,  500.0f, -500.0f,

        -500.0f, -500.0f,  500.0f,
        -500.0f, -500.0f, -500.0f,
        -500.0f,  500.0f, -500.0f,
        -500.0f,  500.0f, -500.0f,
        -500.0f,  500.0f,  500.0f,
        -500.0f, -500.0f,  500.0f,

         500.0f, -500.0f, -500.0f,
         500.0f, -500.0f,  500.0f,
         500.0f,  500.0f,  500.0f,
         500.0f,  500.0f,  500.0f,
         500.0f,  500.0f, -500.0f,
         500.0f, -500.0f, -500.0f,

        -500.0f, -500.0f,  500.0f,
        -500.0f,  500.0f,  500.0f,
         500.0f,  500.0f,  500.0f,
         500.0f,  500.0f,  500.0f,
         500.0f, -500.0f,  500.0f,
        -500.0f, -500.0f,  500.0f,

        -500.0f,  500.0f, -500.0f,
         500.0f,  500.0f, -500.0f,
         500.0f,  500.0f,  500.0f,
         500.0f,  500.0f,  500.0f,
        -500.0f,  500.0f,  500.0f,
        -500.0f,  500.0f, -500.0f,

        -500.0f, -500.0f, -500.0f,
        -500.0f, -500.0f,  500.0f,
         500.0f, -500.0f, -500.0f,
         500.0f, -500.0f, -500.0f,
        -500.0f, -500.0f,  500.0f,
         500.0f, -500.0f,  500.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void initAsteroids() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> radiusDist(160.0f, 560.0f);
    std::uniform_real_distribution<float> heightDist(-20.0f, 20.0f);
    std::uniform_real_distribution<float> scaleDist(0.17f, 1.0f);
    std::uniform_real_distribution<float> rotSpeedDist(-0.001f, 0.001f);
    std::uniform_real_distribution<float> elevationDist(-75.0f, 75.0f);
    
    asteroids.resize(NUM_ASTEROIDS);
    
    int goldCount = 0;
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        bool validPosition = false;
        glm::vec3 newPosition;
        float newScale = scaleDist(gen);
        
        // 尝试找到一个有效位置，确保与其他小行星有足够距离
        int attempts = 0;
        while (!validPosition && attempts < 100) {
            float azimuth = angleDist(gen);
            float elevation = elevationDist(gen);
            float radius = radiusDist(gen);
            
            // 球面坐标转换为笛卡尔坐标
            newPosition = glm::vec3(
                radius * cos(glm::radians(elevation)) * cos(glm::radians(azimuth)),
                radius * sin(glm::radians(elevation)),
                radius * cos(glm::radians(elevation)) * sin(glm::radians(azimuth))
            );
            
            validPosition = true;
            // 检查与已有小行星的距离
            for (int j = 0; j < i; j++) {
                float distance = glm::length(newPosition - asteroids[j].position);
                float minDistance = (newScale + asteroids[j].scale) * 2.0f;
                if (distance < minDistance) {
                    validPosition = false;
                    break;
                }
            }
            attempts++;
        }
        
        // 如果找不到合适位置，使用随机位置但调整半径
        if (!validPosition) {
            float azimuth = angleDist(gen);
            float elevation = elevationDist(gen);
            float radius = radiusDist(gen) + i * 1.2f;
            newPosition = glm::vec3(
                radius * cos(glm::radians(elevation)) * cos(glm::radians(azimuth)),
                radius * sin(glm::radians(elevation)),
                radius * cos(glm::radians(elevation)) * sin(glm::radians(azimuth))
            );
        }
        
        asteroids[i].position = newPosition;
        asteroids[i].rotation = glm::vec3(angleDist(gen), angleDist(gen), angleDist(gen));
        asteroids[i].rotationSpeed = glm::vec3(rotSpeedDist(gen), rotSpeedDist(gen), rotSpeedDist(gen));
        asteroids[i].scale = newScale;

        asteroids[i].isGold = (goldCount < NUM_GOLD_ASTEROIDS) && (i % 32 == 0);
        asteroids[i].collected = false;
        
        if (asteroids[i].isGold) goldCount++;
    }
}

void resetGame();

void initLocalCrafts() {
    localCrafts.resize(NUM_LOCAL_CRAFTS);
    
    for (int i = 0; i < NUM_LOCAL_CRAFTS; i++) {
        LocalCraft& craft = localCrafts[i];
        
        // 轨道半径更加分散，适应更多飞行器
        craft.orbitRadius = 100.0f + i * 80.0f; // 从160+i*120改为100+i*80，范围更广且更密集
        craft.orbitHeight = 0.0f;
        craft.orbitAngle = i * (360.0f / NUM_LOCAL_CRAFTS); // 均匀分布
        craft.orbitSpeed = 0.09f - (i % 5) * 0.01f; // 速度循环变化：0.09, 0.08, 0.07, 0.06, 0.05

        // 使用更多样的轨道倾斜角，创建3D分布
        craft.orbitInclination = (i % 5) * 20.0f; // 0°, 20°, 40°, 60°, 80° - 五种倾斜角循环
        craft.inclinationSpeed = 0.01f + (i % 3) * 0.005f; // 三种不同的速度
        
        // 根据轨道参数设置初始位置
        float x = craft.orbitRadius * cos(glm::radians(craft.orbitAngle));
        float z = craft.orbitRadius * sin(glm::radians(craft.orbitAngle));
        float y = z * sin(glm::radians(craft.orbitInclination));
        z = z * cos(glm::radians(craft.orbitInclination));
        
        craft.position = glm::vec3(x, y, z);
        craft.direction = glm::vec3(1.0f, 0.0f, 0.0f);
        craft.rotation = craft.orbitAngle + 90.0f;
        craft.speed = 0.05f;
        craft.alerted = false;
    }
}

// 键盘输入回调
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (action == GLFW_PRESS) {
        keys[key] = true;
        
        // 按R键重置游戏
        if (key == GLFW_KEY_R) {
            resetGame();
        }
        
        // 记录作弊码输入
        if (key != lastKey) { // 防止长按按键导致重复记录
            if (key == GLFW_KEY_UP) cheatCode += "u";
            else if (key == GLFW_KEY_DOWN) cheatCode += "d";
            else if (key == GLFW_KEY_LEFT) cheatCode += "l";
            else if (key == GLFW_KEY_RIGHT) cheatCode += "r";
            else if (key == GLFW_KEY_B) cheatCode += "b";
            else if (key == GLFW_KEY_A) cheatCode += "a";
            else cheatCode = ""; // 任何其他键都会重置作弊码
        }
        
        lastKey = key;
        
        // 检查作弊码是否正确
        if (cheatCode == GOLD_CHEAT && !cheatActivated) {
            // 激活作弊：收集所有黄金
            for (auto& asteroid : asteroids) {
                if (asteroid.isGold && !asteroid.collected) {
                    asteroid.collected = true;
                    totalGoldCollected++;
                }
            }
            
            // 更新游戏状态
            gameCompleted = true;
            cheatActivated = true;
            
            std::cout << "CHEAT ACTIVATED: All gold collected! (" << totalGoldCollected << "/" 
                      << NUM_GOLD_ASTEROIDS << ")" << std::endl;
            std::cout << "Spacecraft transformed to gold!" << std::endl;
            
            // 重置作弊码
            cheatCode = "";
        }
        
        // 如果作弊码太长了就截断，防止无限增长
        if (cheatCode.length() > GOLD_CHEAT.length()) {
            cheatCode = cheatCode.substr(cheatCode.length() - GOLD_CHEAT.length());
        }
    } else if (action == GLFW_RELEASE) {
        keys[key] = false;
        
        // 释放按键时清除lastKey，这样当再次按下同一个键时会被记录
        if (key == lastKey) {
            lastKey = -1;
        }
    }
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    // 光照调整控制
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_1) {
            lightIntensity1 += 0.1f;
            std::cout << "Light 1 intensity: " << lightIntensity1 << std::endl;
        }
        if (key == GLFW_KEY_2) {
            lightIntensity1 -= 0.1f;
            if (lightIntensity1 < 0.0f) lightIntensity1 = 0.0f;
            std::cout << "Light 1 intensity: " << lightIntensity1 << std::endl;
        }
        if (key == GLFW_KEY_3) {
            lightIntensity2 += 0.1f;
            std::cout << "Light 2 intensity: " << lightIntensity2 << std::endl;
        }
        if (key == GLFW_KEY_4) {
            lightIntensity2 -= 0.1f;
            if (lightIntensity2 < 0.0f) lightIntensity2 = 0.0f;
            std::cout << "Light 2 intensity: " << lightIntensity2 << std::endl;
        }
    }
}

// 鼠标移动回调
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从上到下递增的
    lastX = xpos;
    lastY = ypos;

    // 计算当前鼠标移动速度
    float currentMouseSpeed = sqrt(xoffset * xoffset + yoffset * yoffset);
    
    // 平滑鼠标移动速度，避免突变
    smoothedMouseSpeed = smoothedMouseSpeed * speedDecayFactor + currentMouseSpeed * (1.0f - speedDecayFactor);
    
    // 简化的灵敏度调整 - 减少非线性效果，提高响应性
    float speedFactor = 1.0f / (1.0f + smoothedMouseSpeed * 0.02f);
    
    
    // 计算动态灵敏度
    float dynamicSensitivity = baseSensitivity * speedFactor;
    
    // 限制灵敏度范围
    dynamicSensitivity = glm::clamp(dynamicSensitivity, minSensitivity, maxSensitivity);

    xoffset *= dynamicSensitivity;
    yoffset *= dynamicSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // 限制俯仰角度，避免过度旋转
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // 更新飞船旋转
    spacecraftRotation.y = glm::radians(yaw);
    spacecraftRotation.x = glm::radians(pitch);
    
    // 更新上一次的鼠标速度
    lastMouseSpeed = currentMouseSpeed;
}

// 修改processInput函数，确保同时支持WASD和方向键
void processInput() {
    if (isCollisionFrozen) {
        return;
    }
    
    float moveSpeed = 0.5f;
    
    // 计算飞船的前进方向向量
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    
    // 计算右方向向量
    glm::vec3 right = glm::normalize(glm::cross(front, cameraUp));
    
    // 默认目标侧倾为0（回正）
    targetRoll = 0.0f;
    
    // 使用WASD控制移动（保持原有功能）
    if (keys[GLFW_KEY_W]) {
        spacecraftPosition += front * moveSpeed; // 前进
    }
    if (keys[GLFW_KEY_S]) {
        spacecraftPosition -= front * moveSpeed; // 后退
    }
    if (keys[GLFW_KEY_A]) {
        spacecraftPosition -= right * moveSpeed; // 左移
        targetRoll = maxRoll;  // 左倾
    }
    if (keys[GLFW_KEY_D]) {
        spacecraftPosition += right * moveSpeed; // 右移
        targetRoll = -maxRoll; // 右倾
    }
    
    // 按课程要求添加方向键控制
    if (keys[GLFW_KEY_UP]) {
        spacecraftPosition += front * moveSpeed; // 前进
    }
    if (keys[GLFW_KEY_DOWN]) {
        spacecraftPosition -= front * moveSpeed; // 后退
    }
    if (keys[GLFW_KEY_LEFT]) {
        spacecraftPosition -= right * moveSpeed; // 左移
        targetRoll = maxRoll;  // 左倾
    }
    if (keys[GLFW_KEY_RIGHT]) {
        spacecraftPosition += right * moveSpeed; // 右移
        targetRoll = -maxRoll; // 右倾
    }
    
    // 平滑过渡到目标侧倾角度
    if (currentRoll < targetRoll) {
        currentRoll += rollSpeed;
        if (currentRoll > targetRoll) currentRoll = targetRoll;
    }
    else if (currentRoll > targetRoll) {
        currentRoll -= rollSpeed;
        if (currentRoll < targetRoll) currentRoll = targetRoll;
    }
}

// 碰撞检测函数
bool checkCollision(const glm::vec3& pos1, float radius1, const glm::vec3& pos2, float radius2) {
    float distance = glm::length(pos1 - pos2);
    return distance < (radius1 + radius2);
}

// 更新游戏逻辑
void updateGameLogic(float deltaTime) {
    // 如果处于冻结状态，更新冻结计时器
    if (isCollisionFrozen) {
        freezeTimer += deltaTime;
        
        // 冻结时间已过，执行重置
        if (freezeTimer >= FREEZE_DURATION && shouldResetGame) {
            resetGame();
        }
        
        return; // 冻结状态下不执行其他游戏逻辑
    }
    
    // 检测与本地飞行器的碰撞
    float spacecraftRadius = 2.0f;
    float localCraftRadius = 3.0f;
    
    bool collidedWithCraft = false;
    
    for (auto& craft : localCrafts) {
        if (checkCollision(spacecraftPosition, spacecraftRadius, craft.position, localCraftRadius)) {
            craft.alerted = true;
            alertTriggered = true;
            collidedWithCraft = true;  // 标记已发生碰撞
        }
    }
    
    // 如果发生碰撞，启动冻结模式，并标记需要重置
    if (collidedWithCraft) {
        isCollisionFrozen = true;
        freezeTimer = 0.0f;
        shouldResetGame = true;
        std::cout << "Collision with local craft! Game will reset in 1 second..." << std::endl;
    }
    
    // 检测与黄金小行星的碰撞
    float asteroidRadius = 2.0f;
    
    for (auto& asteroid : asteroids) {
        if (asteroid.isGold && !asteroid.collected) {
            if (checkCollision(spacecraftPosition, spacecraftRadius, asteroid.position, asteroidRadius)) {
                asteroid.collected = true;
                totalGoldCollected++;
                std::cout << "Gold collected! Total: " << totalGoldCollected << "/" << NUM_GOLD_ASTEROIDS << std::endl;
                
                // 检查是否收集完所有黄金
                if (totalGoldCollected >= NUM_GOLD_ASTEROIDS) {
                    gameCompleted = true;
                    std::cout << "Congratulations! All gold collected! Spacecraft turned to gold!" << std::endl;
                }
            }
        }
    }
}

void sendDataToOpenGL()
{
    shader = new Shader();
    shader->setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
    
    skyboxShader = new Shader();
    skyboxShader->setupShader("SkyboxVertexShader.glsl", "SkyboxFragmentShader.glsl");
    
    // 加载模型
    planetModel = loadOBJ("../object/planet.obj");
    spacecraftModel = loadOBJ("../object/spacecraft.obj");
    rockModel = loadOBJ("../object/rock.obj");
    craftModel = loadOBJ("../object/craft.obj");
    
    // 设置模型VAO
    setupModel(planetVAO, planetVBO, planetEBO, planetModel);
    setupModel(spacecraftVAO, spacecraftVBO, spacecraftEBO, spacecraftModel);
    setupModel(rockVAO, rockVBO, rockEBO, rockModel);
    setupModel(craftVAO, craftVBO, craftEBO, craftModel);
    
    // 加载纹理并验证
    std::cout << "Loading textures..." << std::endl;
    
    planetTexture.setupTexture("../texture/earthTexture.bmp");
    planetNormalMap.setupTexture("../texture/earthNormal.bmp");
    spacecraftTexture.setupTexture("../texture/spacecraftTexture.bmp");
    spacecraftGoldTexture.setupTexture("../texture/gold.bmp");
    rockTexture.setupTexture("../texture/rockTexture.bmp");
    goldTexture.setupTexture("../texture/gold.bmp");
    craftTexture.setupTexture("../texture/spacecraftTexture.bmp");
    craftAlertTexture.setupTexture("../texture/red.bmp");
    
    std::cout << "All textures loaded successfully!" << std::endl;
    
    // 设置skybox
    setupSkybox();
    
    // 加载skybox纹理
    std::vector<std::string> faces = {
        "skybox textures/right.bmp",
        "skybox textures/left.bmp",
        "skybox textures/top.bmp",
        "skybox textures/bottom.bmp",
        "skybox textures/front.bmp",
        "skybox textures/back.bmp"
    };
    Texture skyboxLoader;
    skyboxTexture = skyboxLoader.loadSkybox(faces);
    
    // 初始化场景对象
    initAsteroids();
    initLocalCrafts();
}

void initializedGL(void)
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }

    get_OpenGL_info();
    sendDataToOpenGL();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void updateCamera() {
    // 计算飞船的朝向向量（基于当前的yaw和pitch）
    glm::vec3 spacecraftFront;
    spacecraftFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    spacecraftFront.y = sin(glm::radians(pitch));
    spacecraftFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    spacecraftFront = glm::normalize(spacecraftFront);
    
    // 计算飞船的右方向和上方向
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 spacecraftRight = glm::normalize(glm::cross(spacecraftFront, worldUp));
    glm::vec3 spacecraftUp = glm::normalize(glm::cross(spacecraftRight, spacecraftFront));
    
    // 让上方向受侧倾角影响
    glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(currentRoll * 0.5f), spacecraftFront);
    glm::vec4 rolledUp = rollMatrix * glm::vec4(spacecraftUp, 0.0f);
    spacecraftUp = glm::normalize(glm::vec3(rolledUp));
    
    // 摄像机相对于飞船的固定偏移量
    float cameraDistance = 15.0f;  // 后方距离
    float cameraHeight = 6.0f;     // 上方高度
    
    // 计算摄像机位置（始终在飞船的后上方）
    cameraPos = spacecraftPosition - spacecraftFront * cameraDistance + spacecraftUp * cameraHeight;
    
    // 摄像机朝向：看向飞船前方的目标点
    glm::vec3 targetPos = spacecraftPosition + spacecraftFront * 10.0f;
    cameraFront = glm::normalize(targetPos - cameraPos);
    
    // 摄像机的上方向与飞船保持一致
    cameraUp = spacecraftUp;
}

void updateScene(float deltaTime) {
    // 处理输入
    processInput();
    
    // 更新游戏逻辑，传入deltaTime
    updateGameLogic(deltaTime);
    
    // 如果处于冻结状态，不更新其他场景元素
    if (isCollisionFrozen) {
        updateCamera(); // 仍然更新摄像机，确保视图正常
        return;
    }
    
    // 更新行星自转 - 缓慢旋转
    planetRotation += 0.0002f;
    
    // 更新小行星旋转
    for (auto& asteroid : asteroids) {
        asteroid.rotation += asteroid.rotationSpeed;
    }
    
    // 更新本地太空飞行器公转
    static float globalTime = 0.0f;
    globalTime += 0.005f; // 减慢全局时间增长速度
    
    for (int i = 0; i < localCrafts.size(); i++) {
        auto& craft = localCrafts[i];
        
        // 更新公转角度
        craft.orbitAngle += craft.orbitSpeed;
        if (craft.orbitAngle >= 360.0f) {
            craft.orbitAngle -= 360.0f;
        }
        
        // 缓慢变化的轨道倾斜角
        float currentInclination = craft.orbitInclination + sin(globalTime * craft.inclinationSpeed) * 5.0f; // 减小变化幅度
        
        // 根据公转角度和倾斜角更新位置
        float x = craft.orbitRadius * cos(glm::radians(craft.orbitAngle));
        float z_base = craft.orbitRadius * sin(glm::radians(craft.orbitAngle));
        float y = z_base * sin(glm::radians(currentInclination));
        float z = z_base * cos(glm::radians(currentInclination));
        
        craft.position = glm::vec3(x, y, z);
        
        // 更新飞行器朝向（切线方向，考虑倾斜）
        craft.rotation = craft.orbitAngle + 90.0f;
    }
    
    updateCamera();
}

void renderSkybox(const glm::mat4& view, const glm::mat4& projection) {
    glDepthFunc(GL_LEQUAL);
    skyboxShader->use();
    
    // 只保留旋转部分，移除平移，这是标准的天空盒渲染技术
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    skyboxShader->setMat4("view", skyboxView);
    skyboxShader->setMat4("projection", projection);
    
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    skyboxShader->setInt("skybox", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void renderPlanet(const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, planetPosition);
    model = glm::rotate(model, planetRotation, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(30.0f));
    
    shader->setMat4("model", model);
    
    planetTexture.bind(0);
    planetNormalMap.bind(1);
    shader->setInt("diffuseTexture", 0);
    shader->setInt("normalMap", 1);
    shader->setBool("useNormalMap", true);
    
    glBindVertexArray(planetVAO);
    glDrawElements(GL_TRIANGLES, planetModel.indices.size(), GL_UNSIGNED_INT, 0);
}

void renderSpacecraft(const glm::mat4& view, const glm::mat4& projection) {
    // 获取与摄像机一致的朝向向量
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    
    // 使用LookAt矩阵计算飞行器的朝向
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, spacecraftPosition); // 首先移动到飞行器位置
    
    // 创建一个朝向矩阵
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(front, up));
    up = glm::normalize(glm::cross(right, front));
    
    // 构建基于摄像机方向的旋转矩阵
    glm::mat4 rotationMatrix(
        glm::vec4(right, 0.0f),
        glm::vec4(up, 0.0f),
        glm::vec4(-front, 0.0f), // 注意负号，因为模型应该面向前方
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    
    // 应用基于摄像机的旋转
    model = model * rotationMatrix;
    
    // 添加侧倾效果（围绕前进方向的Z轴旋转）
    model = glm::rotate(model, glm::radians(currentRoll), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // 额外添加顺时针旋转90度（在模型的局部坐标系中是绕Y轴旋转-90度）
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // 应用模型特定的修正旋转，确保模型朝向正确
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y轴旋转90度修正
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z轴旋转180度修正
    
    // 调整飞船的大小
    model = glm::scale(model, glm::vec3(0.008f));
    
    shader->setMat4("model", model);
    shader->setBool("useNormalMap", false);
    
    // 修复纹理绑定顺序 - 先设置uniform再绑定纹理
    shader->setInt("diffuseTexture", 0);
    
    if (gameCompleted || cheatActivated) {
        glActiveTexture(GL_TEXTURE0);
        spacecraftGoldTexture.bind(0);
        std::cout << "DEBUG: Using gold texture for spacecraft" << std::endl; // 调试信息
    } else {
        glActiveTexture(GL_TEXTURE0);
        spacecraftTexture.bind(0);
    }
    
    glBindVertexArray(spacecraftVAO);
    glDrawElements(GL_TRIANGLES, spacecraftModel.indices.size(), GL_UNSIGNED_INT, 0);
}

void renderAsteroids(const glm::mat4& view, const glm::mat4& projection) {
    shader->setBool("useNormalMap", false);
    
    for (const auto& asteroid : asteroids) {
        // 跳过已收集的黄金小行星
        if (asteroid.isGold && asteroid.collected) {
            continue;
        }
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, asteroid.position);
        model = glm::rotate(model, asteroid.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        
        if (asteroid.isGold) {
            // 黄金小行星扩大3倍
            model = glm::scale(model, glm::vec3(asteroid.scale * 3.0f));
        } else {
            // 普通岩石扩大2倍
            model = glm::scale(model, glm::vec3(asteroid.scale * 2.0f));
        }
        
        shader->setMat4("model", model);
        
        // 修复纹理绑定顺序
        shader->setInt("diffuseTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        
        if (asteroid.isGold) {
            goldTexture.bind(0);
            // 移除调试信息以减少输出
        } else {
            rockTexture.bind(0);
        }
        
        glBindVertexArray(rockVAO);
        glDrawElements(GL_TRIANGLES, rockModel.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

void renderLocalCrafts(const glm::mat4& view, const glm::mat4& projection) {
    shader->setBool("useNormalMap", false);
    glDepthFunc(GL_LESS);
    
    for (const auto& craft : localCrafts) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, craft.position);
        model = glm::rotate(model, glm::radians(craft.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        
        shader->setMat4("model", model);
        
        // 修复纹理绑定顺序
        shader->setInt("diffuseTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        
        // 根据警报状态选择纹理
        if (craft.alerted) {
            craftAlertTexture.bind(0); // 红色警报纹理
            // 移除调试信息以减少输出
        } else {
            craftTexture.bind(0); // 正常纹理
        }
        
        glBindVertexArray(craftVAO);
        glDrawElements(GL_TRIANGLES, craftModel.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

// 修复paintGL函数中的纹理单元设置
void paintGL(void)
{
    // 计算帧间时间差
    static float lastFrameTime = 0.0f;
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    
    // 使用计算出的deltaTime更新场景
    updateScene(deltaTime);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    
    // 渲染skybox
    renderSkybox(view, projection);
    
    // 使用主着色器
    shader->use();
    
    // 设置光照参数
    shader->setVec3("light1.position", lightPosition1);
    shader->setVec3("light1.ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("light1.diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("light1.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("light1.intensity", lightIntensity1);
    
    shader->setVec3("light2.position", lightPosition2);
    shader->setVec3("light2.ambient", 0.1f, 0.1f, 0.2f);
    shader->setVec3("light2.diffuse", 0.6f, 0.6f, 0.8f);
    shader->setVec3("light2.specular", 0.8f, 0.8f, 1.0f);
    shader->setFloat("light2.intensity", lightIntensity2);
    
    shader->setVec3("viewPos", cameraPos);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    shader->setInt("diffuseTexture", 0);
    shader->setInt("normalMap", 1);
    
    // 渲染所有对象
    renderAsteroids(view, projection);
    renderLocalCrafts(view, projection);
    renderPlanet(view, projection);
    renderSpacecraft(view, projection);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cleanUp() {
    delete shader;
    delete skyboxShader;
    
    glDeleteVertexArrays(1, &planetVAO);
    glDeleteBuffers(1, &planetVBO);
    glDeleteBuffers(1, &planetEBO);
    
    glDeleteVertexArrays(1, &spacecraftVAO);
    glDeleteBuffers(1, &spacecraftVBO);
    glDeleteBuffers(1, &spacecraftEBO);
    
    glDeleteVertexArrays(1, &rockVAO);
    glDeleteBuffers(1, &rockVBO);
    glDeleteBuffers(1, &rockEBO);
    
    glDeleteVertexArrays(1, &craftVAO);
    glDeleteBuffers(1, &craftVBO);
    glDeleteBuffers(1, &craftEBO);
    
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    
    glDeleteTextures(1, &skyboxTexture);
}

void resetGame() {
    // 重置飞船位置和旋转
    spacecraftPosition = glm::vec3(0.0f, 0.0f, 600.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    spacecraftRotation = glm::vec3(0.0f, 0.0f, glm::radians(180.0f));
    
    // 重置所有黄金小行星为未收集状态
    for (auto& asteroid : asteroids) {
        if (asteroid.isGold) {
            asteroid.collected = false;
        }
    }
    
    // 重置游戏状态变量
    totalGoldCollected = 0;
    gameCompleted = false;
    cheatActivated = false;
    alertTriggered = false;
    
    // 重置所有本地飞行器为未警报状态
    for (auto& craft : localCrafts) {
        craft.alerted = false;
    }
    
    // 重置冻结状态
    isCollisionFrozen = false;
    freezeTimer = 0.0f;
    shouldResetGame = false;
    
    // 清空作弊码
    cheatCode = "";
    
    std::cout << "Game Reset! Try to collect all " << NUM_GOLD_ASTEROIDS << " gold asteroids." << std::endl;
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space Gold Collector", NULL, NULL);
    if (!window) {
        const char* description;
        glfwGetError(&description);
        std::cerr << "Failed to create GLFW window: " << (description ? description : "Unknown error") << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // 捕获鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Using OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "=== Space Gold Collector ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- Mouse: Control spacecraft orientation" << std::endl;
    std::cout << "- W/A/S/D or Arrow Keys: Move spacecraft" << std::endl;
    std::cout << "- R: Reset game" << std::endl;
    std::cout << "- 1/2: Adjust Light 1 intensity" << std::endl;
    std::cout << "- 3/4: Adjust Light 2 intensity" << std::endl;
    std::cout << "- ESC: Exit" << std::endl;
    std::cout << "Objective: Collect all " << NUM_GOLD_ASTEROIDS << " gold asteroids!" << std::endl;
    std::cout << "Avoid local crafts - they will turn red if alerted!" << std::endl;
    std::cout << "=============================" << std::endl;

    initializedGL();
    
    // 确保飞船的Z轴旋转正确，使其朝向正确（恢复原始设置）
    spacecraftRotation.z = glm::radians(180.0f);
    
    while (!glfwWindowShouldClose(window)) {
        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUp();
    glfwTerminate();

    return 0;
}
