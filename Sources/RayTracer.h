// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Image.h"
#include "Scene.h"
#include "Ray.h"
#include "Console.h"
#include "Camera.h"
#include "PBR.h"
#include "Solid3DNoise.h"

using namespace std;


class RayTracer {
public:
	
	RayTracer();
	virtual ~RayTracer();

	inline void setResolution (int width, int height) { m_imagePtr = make_shared<Image> (width, height); }
	inline std::shared_ptr<Image> image () { return m_imagePtr; }
	void init (const std::shared_ptr<Scene> scenePtr);
	void activateBVH(bool state) { BVHisActive = state; }
	void render (const std::shared_ptr<Scene> scenePtr);
	inline std::shared_ptr<RayHit> rayScene(const std::shared_ptr<Ray>& ray, const std::shared_ptr<Scene> scenePtr);
	glm::vec3 lightRadiance(const std::shared_ptr<LightSource>& lightPtr, const glm::vec3& position) const;
	glm::vec3 materialReflectance(const std::shared_ptr<Scene> scenePtr,
		const std::shared_ptr<Material> material,
		const glm::vec3& wi,
		const glm::vec3& wo,
		const glm::vec2& uv, 
		const glm::vec3& n) const;
	glm::vec3 shade(const std::shared_ptr<Scene> scenePtr, const std::shared_ptr<RayHit>& rayHit, const std::shared_ptr<Ray> ray);
	std::shared_ptr<Ray> rayAt(float x, float y, const glm::mat4& frameMatrix, const std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Image> m_imagePtr;
	bool BVHisActive;
};