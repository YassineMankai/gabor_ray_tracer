// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#include <iostream>
#include <vector>

#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(int id, const std::string& filename, bool floatingPoint)
			:m_id(id), m_filename(filename), m_isFloatingPoint(floatingPoint){
	std::cout << "Start Loading texture "  << filename << std::endl;
	
	GLuint texID;
	
	if (!floatingPoint) {
		// Loading the image in CPU memory using stbd_image
		unsigned char* data = stbi_load(filename.c_str(),
			&m_width,
			&m_height,
			&m_nbComponent, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
			0);
		// Create a texture in GPU memory
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Uploading the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			m_width,
			m_height,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			GL_UNSIGNED_BYTE,
			data);
		// Generating mipmaps for filtered texture fetch
		glGenerateMipmap(GL_TEXTURE_2D);
		// keeping the CPU memory for cpu based raytracing
		m_data = (void*) data;
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		// Loading the image in CPU memory using stbd_image
		float* data = stbi_loadf(filename.c_str(),
			&m_width,
			&m_height,
			&m_nbComponent,
			0);
		// Create a texture in GPU memory
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Uploading the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(m_nbComponent == 1 ? GL_R16F : (m_nbComponent == 3 ? GL_RGB16F : GL_RGBA16F)),
			m_width,
			m_height,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			GL_FLOAT,
			data);
		// Generating mipmaps for filtered texture fetch
		glGenerateMipmap(GL_TEXTURE_2D);
		// keeping the CPU memory for cpu based raytracing
		m_data = (void*) data;
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	m_contextId = texID;
}

Texture::Texture(int id, int width, int height, int nbComponent, const float* data, bool floatingPoint)
	:m_id(id), m_width(width), m_height(height), m_nbComponent(nbComponent), m_filename(""), m_isFloatingPoint(floatingPoint) {

	GLuint texID;

	if (!floatingPoint) {
		// Create a texture in GPU memory
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Uploading the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			m_width,
			m_height,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			GL_UNSIGNED_BYTE,
			data);
		// Generating mipmaps for filtered texture fetch
		glGenerateMipmap(GL_TEXTURE_2D);
		// keeping the CPU memory for cpu based raytracing
		m_data = (void*) data;
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		// Create a texture in GPU memory
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Uploading the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D,
			0,
			(m_nbComponent == 1 ? GL_R16F : (m_nbComponent == 3 ? GL_RGB16F : GL_RGBA16F)),
			m_width,
			m_height,
			0,
			(m_nbComponent == 1 ? GL_RED : (m_nbComponent == 3 ? GL_RGB : GL_RGBA)),
			GL_FLOAT,
			data);
		// Generating mipmaps for filtered texture fetch
		glGenerateMipmap(GL_TEXTURE_2D);
		// keeping the CPU memory for cpu based raytracing
		m_data = (void *) data;
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	m_contextId = texID;
}

Texture::~Texture() {
	if (m_filename == "") {
		delete[] ((float *)m_data);
	}
	else {
		stbi_image_free(m_data);
	}
}

glm::vec3 Texture::fetch(int x, int y) {
	int index = (x + y * m_width) * m_nbComponent;
	if (!m_isFloatingPoint) {
		unsigned char* pixel_ptr = (unsigned char*) m_data + index;
		if (m_nbComponent >= 3) {
			return glm::vec3(float(pixel_ptr[0]) / 255, float(pixel_ptr[1]) / 255, float(pixel_ptr[2]) / 255);
		}
		else
		{
			return glm::vec3(float(pixel_ptr[0]) / 255, 0, 0);
		}
	}
	else {
		float* pixel_ptr = (float*) m_data + index;
		if (m_nbComponent >= 3) {
			return glm::vec3(pixel_ptr[0], pixel_ptr[1], pixel_ptr[2]);
		}
		else
		{
			return glm::vec3(pixel_ptr[0], 0, 0);
		}
	}
}

glm::vec3 Texture::fetch(glm::vec2 uv_input) {
	glm::vec2 uv = glm::vec2(fmod(uv_input.x, 1), fmod(uv_input.y, 1));
	int x_left_bottom = static_cast<int>(uv.x * (m_width-1));
	int y_left_bottom = static_cast<int>(uv.y * (m_height - 1));
	
	int x_left_top = x_left_bottom;
	int y_left_top = y_left_bottom + 1;

	int x_right_bottom = x_left_bottom + 1;
	int y_right_bottom = y_left_bottom;
		  
	int x_right_top = x_left_bottom + 1;
	int y_right_top = y_left_bottom + 1;

	float alpha = uv.x * (m_width - 1) - x_left_bottom;
	float beta = uv.y * (m_height - 1) - y_left_bottom;

	glm::vec3 pixel_left_bottom = fetch(x_left_bottom, y_left_bottom);
	glm::vec3 pixel_left_top = fetch(x_left_top, y_left_top);
	glm::vec3 pixel_right_bottom = fetch(x_right_bottom, y_right_bottom);
	glm::vec3 pixel_right_top = fetch(x_right_top, y_right_top);

	glm::vec3 pixel_left = beta * pixel_left_top + (1 - beta) * pixel_left_bottom;
	glm::vec3 pixel_right = beta * pixel_right_top + (1 - beta) * pixel_right_bottom;

	return alpha * pixel_right + (1 - alpha) * pixel_left;
}