/*
Student Information
Student ID:
Student Name:
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <map>
#include <deque>
#include <random>

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

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
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
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
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
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
Shader* shadowShader;
Shader* particleShader;
Shader* trailShader;
Model penguinModel, snowfieldModel;

// 模型VAO
GLuint penguinVAO, penguinVBO, penguinEBO;
GLuint snowfieldVAO, snowfieldVBO, snowfieldEBO;
GLuint depthMapFBO;
GLuint depthMap;
GLuint particleVAO, particleVBO;
GLuint trailVAO, trailVBO;

// 纹理对象
Texture penguinTexture1, penguinTexture2;
Texture snowfieldTexture1, snowfieldTexture2;

// 光照参数
float directionalLightIntensity = 0.8f;
glm::vec3 directionalLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
bool showShadows = true;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

// 企鹅控制参数
glm::vec3 penguinPosition = glm::vec3(0.0f, 0.5f, 0.0f);
float penguinRotation = 0.0f;
std::deque<glm::vec3> penguinTrail;
const int MAX_TRAIL_POINTS = 100;
bool showTrail = true;

// 雪花粒子系统
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float size;
};
std::vector<Particle> particles;
const int MAX_PARTICLES = 500;
bool showSnow = true;

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 鼠标控制参数
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;
bool leftMousePressed = false;

// 纹理选择
int currentPenguinTexture = 1;
int currentSnowfieldTexture = 1;

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

void initParticles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(-15.0f, 15.0f);
    std::uniform_real_distribution<float> zDist(-15.0f, 15.0f);
    std::uniform_real_distribution<float> yDist(0.0f, 15.0f);
    std::uniform_real_distribution<float> velDist(-0.01f, 0.01f);
    std::uniform_real_distribution<float> lifeDist(0.5f, 1.0f);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.15f);
    
    particles.resize(MAX_PARTICLES);
    
    for (auto& p : particles) {
        p.position = glm::vec3(xDist(gen), yDist(gen), zDist(gen));
        p.velocity = glm::vec3(velDist(gen), -0.05f - velDist(gen) * 0.1f, velDist(gen));
        p.life = lifeDist(gen);
        p.size = sizeDist(gen);
    }
    
    float particleQuad[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };
    
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleQuad), particleQuad, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void updateParticles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(-15.0f, 15.0f);
    std::uniform_real_distribution<float> zDist(-15.0f, 15.0f);
    std::uniform_real_distribution<float> velDist(-0.01f, 0.01f);
    std::uniform_real_distribution<float> lifeDist(0.8f, 1.0f);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.15f);
    
    for (auto& p : particles) {
        p.life -= 0.005f;
        
        if (p.life <= 0.0f || p.position.y <= 0.0f) {
            p.position = glm::vec3(xDist(gen), 15.0f, zDist(gen));
            p.velocity = glm::vec3(velDist(gen), -0.05f - velDist(gen) * 0.1f, velDist(gen));
            p.life = lifeDist(gen);
            p.size = sizeDist(gen);
        }
        
        p.position += p.velocity;
    }
}

void setupShadowMap() {
    glGenFramebuffers(1, &depthMapFBO);
    
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void sendDataToOpenGL()
{
    shader = new Shader();
    shader->setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
    
    shadowShader = new Shader();
    shadowShader->setupShader("ShadowVertexShader.glsl", "ShadowFragmentShader.glsl");
    
    particleShader = new Shader();
    particleShader->setupShader("ParticleVertexShader.glsl", "ParticleFragmentShader.glsl");
    
    trailShader = new Shader();
    trailShader->setupShader("TrailVertexShader.glsl", "TrailFragmentShader.glsl");
    
    penguinModel = loadOBJ("resources/penguin/penguin.obj");
    snowfieldModel = loadOBJ("resources/snow/snow.obj");
    
    setupModel(penguinVAO, penguinVBO, penguinEBO, penguinModel);
    setupModel(snowfieldVAO, snowfieldVBO, snowfieldEBO, snowfieldModel);
    
    penguinTexture1.setupTexture("resources/penguin/penguin_01.png");
    penguinTexture2.setupTexture("resources/penguin/penguin_02.png");
    snowfieldTexture1.setupTexture("resources/snow/snow_01.jpg");
    snowfieldTexture2.setupTexture("resources/snow/snow_02.jpg");
    
    setupShadowMap();
    initParticles();
    
    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);
    
    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_TRAIL_POINTS * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void initializedGL(void)
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void renderScene(Shader* currentShader, bool shadowPass = false) {
    // 渲染雪原
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    currentShader->setMat4("model", model);
    
    if (!shadowPass) {
        if (currentSnowfieldTexture == 1) {
            snowfieldTexture1.bind(0);
        } else {
            snowfieldTexture2.bind(0);
        }
        currentShader->setInt("textureSampler", 0);
    }
    
    glBindVertexArray(snowfieldVAO);
    glDrawElements(GL_TRIANGLES, snowfieldModel.indices.size(), GL_UNSIGNED_INT, 0);
    
    // 渲染企鹅
    model = glm::mat4(1.0f);
    model = glm::translate(model, penguinPosition);
    model = glm::rotate(model, penguinRotation, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    currentShader->setMat4("model", model);
    
    if (!shadowPass) {
        if (currentPenguinTexture == 1) {
            penguinTexture1.bind(0);
        } else {
            penguinTexture2.bind(0);
        }
        currentShader->setInt("textureSampler", 0);
    }
    
    glBindVertexArray(penguinVAO);
    glDrawElements(GL_TRIANGLES, penguinModel.indices.size(), GL_UNSIGNED_INT, 0);
}

void renderTrail(const glm::mat4& view, const glm::mat4& projection) {
    if(penguinTrail.size() < 2 || !showTrail) return;
    
    trailShader->use();
    trailShader->setMat4("view", view);
    trailShader->setMat4("projection", projection);
    trailShader->setVec3("trailColor", 1.0f, 0.5f, 0.0f);
    
    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    
    size_t dataSize = penguinTrail.size() * sizeof(glm::vec3);
    std::vector<glm::vec3> trailPoints;
    
    for(const auto& point : penguinTrail) {
        trailPoints.push_back(glm::vec3(point.x, point.y + 0.05f, point.z));
    }
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, trailPoints.data());
    
    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_STRIP, 0, penguinTrail.size());
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}

void renderParticles(const glm::mat4& view, const glm::mat4& projection) {
    if(!showSnow) return;
    
    updateParticles();
    
    particleShader->use();
    particleShader->setMat4("view", view);
    particleShader->setMat4("projection", projection);
    
    glBindVertexArray(particleVAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    
    for(const auto& p : particles) {
        if(p.life > 0.0f) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, p.position);
            model = glm::scale(model, glm::vec3(p.size));
            
            particleShader->setMat4("model", model);
            particleShader->setFloat("life", p.life);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void paintGL(void)
{
    // 更新企鹅轨迹
    if(showTrail) {
        penguinTrail.push_back(penguinPosition);
        if(penguinTrail.size() > MAX_TRAIL_POINTS) {
            penguinTrail.pop_front();
        }
    }
    
    // 1. 首先渲染阴影贴图
    if(showShadows) {
        const float near_plane = 1.0f, far_plane = 25.0f;
        glm::mat4 lightProjection = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(-directionalLightDir * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        
        shadowShader->use();
        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        renderScene(shadowShader, true);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    // 2. 正常渲染场景
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    
    // 设置光照参数
    // 定向光
    shader->setVec3("dirLight_direction", directionalLightDir);
    shader->setVec3("dirLight_ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("dirLight_diffuse", 0.5f, 0.5f, 0.5f);
    shader->setVec3("dirLight_specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("dirLight_intensity", directionalLightIntensity);
    
    // 点光源
    shader->setVec3("pointLight_position", 2.0f, 2.0f, 2.0f);
    shader->setVec3("pointLight_ambient", 0.1f, 0.1f, 0.1f);
    shader->setVec3("pointLight_diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLight_specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLight_constant", 1.0f);
    shader->setFloat("pointLight_linear", 0.09f);
    shader->setFloat("pointLight_quadratic", 0.032f);
    
    // 添加聚光灯
    shader->setVec3("spotLight_position", penguinPosition + glm::vec3(0.0f, 1.0f, 0.0f));
    shader->setVec3("spotLight_direction", glm::vec3(cos(penguinRotation), -0.5f, sin(penguinRotation)));
    shader->setVec3("spotLight_ambient", 0.0f, 0.0f, 0.0f);
    shader->setVec3("spotLight_diffuse", 1.0f, 1.0f, 0.8f);
    shader->setVec3("spotLight_specular", 1.0f, 1.0f, 0.8f);
    shader->setFloat("spotLight_cutOff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight_outerCutOff", glm::cos(glm::radians(17.5f)));
    shader->setFloat("spotLight_constant", 1.0f);
    shader->setFloat("spotLight_linear", 0.09f);
    shader->setFloat("spotLight_quadratic", 0.032f);
    shader->setFloat("spotLight_intensity", 1.0f);
    
    // 视角位置
    shader->setVec3("viewPos", cameraPos);
    shader->setInt("showShadows", showShadows ? 1 : 0);
    
    // 阴影映射相关
    if(showShadows) {
        const float near_plane = 1.0f, far_plane = 25.0f;
        glm::mat4 lightProjection = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(-directionalLightDir * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        
        shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shader->setInt("shadowMap", 1);
    }
    
    // 设置投影矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->setMat4("projection", projection);
    
    // 设置视图矩阵
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader->setMat4("view", view);
    
    renderScene(shader);
    renderTrail(view, projection);
    renderParticles(view, projection);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
            firstMouse = true;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (leftMousePressed) {
        if (firstMouse) {
            lastX = x;
            lastY = y;
            firstMouse = false;
        }
        
        float xoffset = x - lastX;
        float yoffset = lastY - y;
        lastX = x;
        lastY = y;
        
        const float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;
        
        cameraPos.y += yoffset;
        
        if (cameraPos.y < 0.5f) cameraPos.y = 0.5f;
        if (cameraPos.y > 5.0f) cameraPos.y = 5.0f;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    float cameraSpeed = 0.5f;
    if (yoffset > 0) {
        cameraPos += cameraFront * cameraSpeed;
    } else {
        cameraPos -= cameraFront * cameraSpeed;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) {
            directionalLightIntensity += 0.1f;
            if (directionalLightIntensity > 1.5f) directionalLightIntensity = 1.5f;
        }
        if (key == GLFW_KEY_S) {
            directionalLightIntensity -= 0.1f;
            if (directionalLightIntensity < 0.1f) directionalLightIntensity = 0.1f;
        }
        
        if (key == GLFW_KEY_1) {
            currentPenguinTexture = 1;
        }
        if (key == GLFW_KEY_2) {
            currentPenguinTexture = 2;
        }
        if (key == GLFW_KEY_3) {
            currentSnowfieldTexture = 1;
        }
        if (key == GLFW_KEY_4) {
            currentSnowfieldTexture = 2;
        }
        
        float movementSpeed = 0.1f;
        if (key == GLFW_KEY_UP) {
            penguinPosition.z -= movementSpeed * cos(penguinRotation);
            penguinPosition.x += movementSpeed * sin(penguinRotation);
        }
        if (key == GLFW_KEY_DOWN) {
            penguinPosition.z += movementSpeed * cos(penguinRotation);
            penguinPosition.x -= movementSpeed * sin(penguinRotation);
        }
        if (key == GLFW_KEY_LEFT) {
            penguinRotation += 0.1f;
        }
        if (key == GLFW_KEY_RIGHT) {
            penguinRotation -= 0.1f;
        }
        
        if (key == GLFW_KEY_F1) {
            showShadows = !showShadows;
        }
        if (key == GLFW_KEY_F2) {
            showTrail = !showTrail;
            if (!showTrail) penguinTrail.clear();
        }
        if (key == GLFW_KEY_F3) {
            showSnow = !showSnow;
        }
    }
}

void cleanUp() {
    delete shader;
    delete shadowShader;
    delete particleShader;
    delete trailShader;
    
    glDeleteVertexArrays(1, &penguinVAO);
    glDeleteBuffers(1, &penguinVBO);
    glDeleteBuffers(1, &penguinEBO);
    
    glDeleteVertexArrays(1, &snowfieldVAO);
    glDeleteBuffers(1, &snowfieldVBO);
    glDeleteBuffers(1, &snowfieldEBO);
    
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    
    glDeleteVertexArrays(1, &trailVAO);
    glDeleteBuffers(1, &trailVBO);
    
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
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
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
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
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}

	std::cout << "Using OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		paintGL();

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	cleanUp();
	glfwTerminate();

	return 0;
}






