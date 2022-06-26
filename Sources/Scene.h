// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "LightSource.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Texture2DNoise.h"
#include "BVH.h"


class Scene {
public:
	inline Scene () : m_backgroundColor (0.f, 0.f ,0.f) {}
	virtual ~Scene() {}

	inline const glm::vec3 & backgroundColor () const { return m_backgroundColor; }

	inline void setBackgroundColor (const glm::vec3 & color) { m_backgroundColor = color; }
 
	inline void set (std::shared_ptr<Camera> camera) { m_camera = camera; }

	//inline const std::shared_ptr<Camera> camera() const { return m_camera; }

	inline std::shared_ptr<Camera> camera() { return m_camera; }

	inline void addMesh (std::shared_ptr<Mesh> mesh) { m_meshes.push_back (mesh); }

	inline void addMaterial (std::shared_ptr<Material> material) { m_materials.push_back (material); }
	
	inline void addTexture (std::shared_ptr<Texture> texture) { m_textures.push_back (texture); }

	inline void addModel (std::shared_ptr<Model> model) { m_models.push_back (model); }

	inline void addLight(std::shared_ptr<LightSource> light) { return m_lights.push_back(light); }
	
	inline size_t numOfMeshes () const { return m_meshes.size (); }

	inline size_t numOfMaterials () const { return m_materials.size (); }
	
	inline size_t numOfTextures () const { return m_textures.size (); }

	inline size_t numOfModels () const { return m_models.size (); }

	inline size_t numOfLights() const { return m_lights.size (); }

	inline size_t numOfTriangles() const { return m_triangles.size (); }
	
	const std::shared_ptr<Mesh>& mesh (size_t index) const { return m_meshes[index]; }
	
	const std::shared_ptr<Material>& material (size_t index) const { return m_materials[index]; }
	
	const std::shared_ptr<Texture>& texture (size_t index) const { return m_textures[index]; }
	
	const std::shared_ptr<Model>& model (size_t index) const { return m_models[index]; }
	
	const std::shared_ptr<LightSource>& light (size_t index) const { return m_lights[index]; }

	const Triangle& triangle (size_t index) const { return m_triangles[index]; }

	const std::vector<Triangle>& triangles () const { return m_triangles; }

	const std::shared_ptr<BVH>& sceneBVH() const { return m_bvh; }
	
	void preprocessScene();
	TextureBundle loadTextureBundle(std::string& materialDirName);
	TextureBundle loadTextureBundle(unsigned int resolution, Texture2Dnoise& noise, const std::vector<glm::vec3>& colorMap);
	TextureBundle loadTextureBundle(int textureType, unsigned int resolution, Texture2Dnoise& noise);

	inline void clear () {
		m_camera.reset ();
		m_models.clear();
		m_meshes.clear ();
		m_materials.clear();
		m_textures.clear();
		m_lights.clear();
	}

private:
	glm::vec3 m_backgroundColor;
	std::shared_ptr<Camera> m_camera;
	std::vector<std::shared_ptr<Mesh> > m_meshes;
	std::vector<std::shared_ptr<Material> > m_materials;
	std::vector<std::shared_ptr<Texture> > m_textures;
	std::vector<std::shared_ptr<Model> > m_models;
	std::vector <std::shared_ptr<LightSource>> m_lights;
	std::vector<Triangle> m_triangles;
	std::shared_ptr<BVH> m_bvh;
};