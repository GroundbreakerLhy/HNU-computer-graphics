#pragma once
#include "../../Dependencies/glew/glew.h"
#include <vector>
#include <string>

class Texture 
{
public:
	void setupTexture(const char* texturePath);
	void bind(unsigned int slot) const;
	void unbind() const;
	
	// Skybox相关函数
	unsigned int loadSkybox(std::vector<std::string> faces);

private:
	unsigned int ID = 0;
	int Width = 0, Height = 0, BPP = 0;
};