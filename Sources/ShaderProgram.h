// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <glad/glad.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "Material.h"
#include "Texture.h"
#include "LightSource.h"

class ShaderProgram {
public:
	/// Create the program. A valid OpenGL context must be active.
	ShaderProgram ();

	virtual ~ShaderProgram ();

	/// Generate a minimal shader program, made of one vertex shader and one fragment shader
	static std::shared_ptr<ShaderProgram> genBasicShaderProgram (const std::string & vertexShaderFilename,
															 	 const std::string & fragmentShaderFilename);

	/// OpenGL identifier of the program
	inline GLuint id () { return m_id; }

	/// Loads and compile a shader from a text file, before attaching it to a program
	void loadShader (GLenum type, const std::string & shaderFilename);

	/// The main GPU program is ready to be handle streams of polygons
	inline void link () { 
		glLinkProgram (m_id); 
		int  success;
		char infoLog[512];
		glGetProgramiv(m_id, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_id, 512, NULL, infoLog);
			std::cout << "ERROR::" << "linking shaders" << "::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	/// Activate the program
	inline void use () { glUseProgram (m_id); }

	/// Desactivate the current program
	inline static void stop () { glUseProgram (0); }

	inline GLuint getLocation (const std::string & name) { use (); return glGetUniformLocation (m_id, name.c_str ()); }

	inline void set (const std::string & name, bool value) { use (); glUniform1i (getLocation (name.c_str ()), value ? 1 : 0); }

	inline void set (const std::string & name, float value) { use (); glUniform1f (getLocation (name.c_str ()), value); }

	inline void set(const std::string& name, int value) { use(); glUniform1i(getLocation(name.c_str()), value); }

	inline void set (const std::string & name, unsigned int value) { use (); glUniform1i (getLocation (name.c_str ()), int (value)); }

	inline void set (const std::string & name, const glm::vec2 & value) { use (); glUniform2fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::vec3 & value) { use (); glUniform3fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::vec4 & value) { use (); glUniform4fv (getLocation (name.c_str ()), 1, glm::value_ptr(value)); }

	inline void set (const std::string & name, const glm::mat4 & value) { use (); glUniformMatrix4fv (getLocation (name.c_str ()), 1, GL_FALSE, glm::value_ptr(value)); }
	
	inline void set (const std::shared_ptr<Material> value) {
		use (); 

		set("material.albedo", value->albedo());
		set("material.roughness", value->roughness());
		set("material.metallicness", value->metallicness());
		
		set("material.hasAlbedoTex", value->hasAlbedoTexture());
		set("material.hasRoughnessTex", value->hasRoughnessTexture());
		set("material.hasMetallicTex", value->hasMetallicTexture());
		set("material.hasAmbiantOcclusionTex", value->hasAmbiantOcclusionTexture());
		
		if (value->hasAlbedoTexture()) {
			set("material.albedoTex", 0);			
		}	
		if (value->hasRoughnessTexture()) {
			set("material.roughnessTex", 1);
		}
		if (value->hasMetallicTexture()) {
			set("material.metallicTex", 2);
		}
		if (value->hasAmbiantOcclusionTexture()) {
			set("material.amibantOcclusionTex", 3);
		}
	}
	
	inline void set (const std::shared_ptr<LightSource> value, glm::mat4 viewMatrix, int lightId) {
		use (); 		
		std::string prefix = "lights[" + std::to_string(lightId) + "].";
		set(prefix + "type", static_cast<int>(value->type()));
		if (value->type() == LightType::DirectionalLight) {
			set(prefix + "info", glm::normalize(glm::vec3(glm::transpose(glm::inverse(viewMatrix)) * glm::vec4(value->forward(), 1.0))));
		}
		else {
			set(prefix + "info", value->attenuation());
		}
		set(prefix + "center", glm::vec3(glm::vec4(value->center(), 1.0)));
		set(prefix + "color", value->color());
		set(prefix + "intensity", value->intensity());
	}
	
	
private:
	/// Loads the content of an ASCII file in a standard C++ string
	std::string file2String (const std::string & filename);

	GLuint m_id = 0; 
};

