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
Model penguinModel, snowfieldModel;

// 模型VAO
GLuint penguinVAO, penguinVBO, penguinEBO;
GLuint snowfieldVAO, snowfieldVBO, snowfieldEBO;

// 纹理对象
Texture penguinTexture1, penguinTexture2;
Texture snowfieldTexture1, snowfieldTexture2;

// 光照参数
float directionalLightIntensity = 0.8f;
glm::vec3 directionalLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);

// 企鹅控制参数
glm::vec3 penguinPosition = glm::vec3(0.0f, 0.5f, 0.0f);
float penguinRotation = 0.0f;

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

void sendDataToOpenGL()
{
    shader = new Shader();
    shader->setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
    
    penguinModel = loadOBJ("resources/penguin/penguin.obj");
    snowfieldModel = loadOBJ("resources/snow/snow.obj");
    
    setupModel(penguinVAO, penguinVBO, penguinEBO, penguinModel);
    setupModel(snowfieldVAO, snowfieldVBO, snowfieldEBO, snowfieldModel);
    
    penguinTexture1.setupTexture("resources/penguin/penguin_01.png");
    penguinTexture2.setupTexture("resources/penguin/penguin_02.png");
    snowfieldTexture1.setupTexture("resources/snow/snow_01.jpg");
    snowfieldTexture2.setupTexture("resources/snow/snow_02.jpg");
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

void paintGL(void)
{
	glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    
    shader->setVec3("dirLight_direction", directionalLightDir);
    shader->setVec3("dirLight_ambient", 0.2f, 0.2f, 0.2f);
    shader->setVec3("dirLight_diffuse", 0.5f, 0.5f, 0.5f);
    shader->setVec3("dirLight_specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("dirLight_intensity", directionalLightIntensity);
    
    shader->setVec3("pointLight_position", 2.0f, 2.0f, 2.0f);
    shader->setVec3("pointLight_ambient", 0.1f, 0.1f, 0.1f);
    shader->setVec3("pointLight_diffuse", 0.8f, 0.8f, 0.8f);
    shader->setVec3("pointLight_specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("pointLight_constant", 1.0f);
    shader->setFloat("pointLight_linear", 0.09f);
    shader->setFloat("pointLight_quadratic", 0.032f);
    
    shader->setVec3("viewPos", cameraPos);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->setMat4("projection", projection);
    
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader->setMat4("view", view);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    shader->setMat4("model", model);
    
    if (currentSnowfieldTexture == 1) {
        snowfieldTexture1.bind(0);
    } else {
        snowfieldTexture2.bind(0);
    }
    shader->setInt("textureSampler", 0);
    
    glBindVertexArray(snowfieldVAO);
    glDrawElements(GL_TRIANGLES, snowfieldModel.indices.size(), GL_UNSIGNED_INT, 0);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, penguinPosition);
    model = glm::rotate(model, penguinRotation, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    shader->setMat4("model", model);
    
    if (currentPenguinTexture == 1) {
        penguinTexture1.bind(0);
    } else {
        penguinTexture2.bind(0);
    }
    shader->setInt("textureSampler", 0);
    
    glBindVertexArray(penguinVAO);
    glDrawElements(GL_TRIANGLES, penguinModel.indices.size(), GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
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
    }
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

	glfwTerminate();

	return 0;
}






