// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Scene.h"
#include "Texture2DNoise.h"

void Scene::preprocessScene() {
	for (int modelIndex = 0; modelIndex < numOfModels(); modelIndex++) {
		std::shared_ptr<Model> currentModel = model(modelIndex);
		std::shared_ptr<Mesh> modelMesh = mesh(currentModel->meshId());
		const std::vector<glm::uvec3>& triangleIndices = modelMesh->triangleIndices();

		const std::vector<glm::vec3>& vertexPositions = modelMesh->vertexPositions();
		const std::vector<glm::vec3>& vertexNormals = modelMesh->vertexNormals();
		const std::vector<glm::vec2>& vertexTexCoords = modelMesh->vertexTexCoords();
		
		const glm::mat4& modelMatrix = currentModel->transform().computeTransformMatrix();
		const glm::mat4& normalMatrix = glm::transpose(glm::inverse(modelMatrix));

		for (int trIndex = 0; trIndex < triangleIndices.size(); trIndex++) {
			const int& p0_index = modelMesh->triangleIndices()[trIndex].x;
			const int& p1_index = modelMesh->triangleIndices()[trIndex].y;
			const int& p2_index = modelMesh->triangleIndices()[trIndex].z;

			glm::vec3 p0 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[p0_index], 1));
			glm::vec3 p1 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[p1_index], 1));
			glm::vec3 p2 = glm::vec3(modelMatrix * glm::vec4(vertexPositions[p2_index], 1));

			glm::vec3 n0 = glm::vec3(normalMatrix * glm::vec4(vertexNormals[p0_index], 1));
			glm::vec3 n1 = glm::vec3(normalMatrix * glm::vec4(vertexNormals[p1_index], 1));
			glm::vec3 n2 = glm::vec3(normalMatrix * glm::vec4(vertexNormals[p2_index], 1));
			
			glm::vec2 uv0 = vertexTexCoords[p0_index];
			glm::vec2 uv1 = vertexTexCoords[p1_index];
			glm::vec2 uv2 = vertexTexCoords[p2_index];



			m_triangles.push_back(Triangle(p0, p1, p2, n0, n1, n2, uv0, uv1, uv2, currentModel->materialId(), modelIndex));
		}
	}
	std::vector<int> indices;
	m_bvh = std::make_shared<BVH>(m_triangles, indices);
}

TextureBundle Scene::loadTextureBundle(std::string& materialDirName) {
	TextureBundle result;
	int firstIndex = numOfTextures();
	result.m_albedoTexId = firstIndex;
	result.m_roughnessTexId = firstIndex + 1;
	result.m_metallicTexId = firstIndex + 2;
	result.m_ambientOcclusionTexId = firstIndex + 3;

	auto albedo = std::make_shared<Texture>(result.m_roughnessTexId, materialDirName + "Base_Color.png", true);
	auto roughness = std::make_shared<Texture>(result.m_roughnessTexId, materialDirName + "Roughness.png", true);
	auto metallic = std::make_shared<Texture>(result.m_metallicTexId, materialDirName + "Metallic.png", true);
	auto ambientOcclusion = std::make_shared<Texture>(result.m_ambientOcclusionTexId, materialDirName + "Ambient_Occlusion.png", true);

	addTexture(albedo);
	addTexture(roughness);
	addTexture(metallic);
	addTexture(ambientOcclusion);

	return result;
}

TextureBundle Scene::loadTextureBundle(unsigned int resolution, Texture2Dnoise& noise, const std::vector<glm::vec3> &colorMap) {
	TextureBundle result;
	result.m_albedoTexId = numOfTextures();
	result.m_roughnessTexId = -1;
	result.m_metallicTexId = -1;
	result.m_ambientOcclusionTexId = -1;
	
	auto texture = noise.generateColor2DNoiseTexture(numOfTextures(), resolution, colorMap);
	addTexture(texture);
	return result;
}

TextureBundle Scene::loadTextureBundle(int textureType, unsigned int resolution, Texture2Dnoise& noise) {
	TextureBundle result;
	int firstIndex = numOfTextures();
	result.m_albedoTexId = -1;
	result.m_roughnessTexId = -1;
	result.m_metallicTexId = -1;
	result.m_ambientOcclusionTexId = -1;
	switch (textureType) {
	case 1:
		result.m_roughnessTexId = firstIndex;
		break;
	case 2:
		result.m_metallicTexId = firstIndex;
		break;
	case 3:
		result.m_ambientOcclusionTexId = firstIndex;
		break;
	}
	auto texture = noise.generateFloat2DNoiseTexture(firstIndex, resolution);
	addTexture(texture);
	
	return result;
}
