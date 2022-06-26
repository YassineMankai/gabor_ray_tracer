#pragma once


#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct TextureBundle {
	int m_albedoTexId;
	int m_roughnessTexId;
	int m_metallicTexId;
	int m_ambientOcclusionTexId;
};

class Texture
{
public:
	Texture(int id, const std::string& filename, bool floatingPoint);
	Texture(int id, int width, int height, int nbComponent, const float * data, bool floatingPoint);
	~Texture();
	GLuint getContextId() {return m_contextId;}
	int getScenetId() {return m_id;}
	glm::vec3 fetch(glm::vec2 uv);

private:
	int m_id; // For the moment, id = index in scene.m_textures  // for denugging purpose
	std::string m_filename;
	GLuint m_contextId;
	bool m_isFloatingPoint;
	int m_width;
	int m_height;
	int m_nbComponent;
	void* m_data;
	glm::vec3 fetch(int x, int y);
};
