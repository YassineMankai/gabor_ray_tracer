// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#include "Material.h"

glm::vec3& Material::albedo() {
	return m_albedo; 
}
float& Material::roughness() {
	return m_roughness;
}
float& Material::metallicness() {
	return m_metallicness;
}

glm::vec3& Material::albedo(const glm::vec3& pos, const glm::vec3& normal) {
	return m_albedo; 
}
float& Material::roughness(const glm::vec3& pos, const glm::vec3& normal) {
	return m_roughness;
}
float& Material::metallicness(const glm::vec3& pos, const glm::vec3& normal) {
	return m_metallicness;
}