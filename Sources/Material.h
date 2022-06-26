#pragma once


#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Texture.h"

class Material
{
public:
	Material(int id, glm::vec3 albedo, float roughness, float metallicness, const TextureBundle& textureBundle = { -1,-1, -1, -1 })
		:m_id(id), m_albedo(albedo), m_roughness(roughness), m_metallicness(metallicness), m_textureBundle(textureBundle) {};
	glm::vec3& albedo();
	float& roughness();
	float& metallicness();
	virtual glm::vec3& albedo(const glm::vec3& pos, const glm::vec3& normal);
	virtual float& roughness(const glm::vec3& pos, const glm::vec3& normal);
	virtual float& metallicness(const glm::vec3& pos, const glm::vec3& normal);
	inline const int& getId() const { return m_id; }
	inline const TextureBundle& getTextureBundle() const { return m_textureBundle; }
	inline const bool hasAlbedoTexture() const { return m_textureBundle.m_albedoTexId != -1; }
	inline const bool hasRoughnessTexture() const { return m_textureBundle.m_roughnessTexId != -1; }
	inline const bool hasMetallicTexture() const { return m_textureBundle.m_metallicTexId != -1; }
	inline const bool hasAmbiantOcclusionTexture() const { return m_textureBundle.m_ambientOcclusionTexId != -1; }
protected:
	int m_id; // id = index in scene.m_materials
	TextureBundle m_textureBundle;
	glm::vec3 m_albedo;
	float m_roughness;
	float m_metallicness;
};