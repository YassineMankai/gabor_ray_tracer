// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#define _USE_MATH_DEFINES

#include "RayTracer.h"

// ### Textures

RayTracer::RayTracer() :
	m_imagePtr(std::make_shared<Image>()), BVHisActive(true) {
	float K_ = 1.0;
	float a_ = 0.1;
	float F_0_ = 0.0625;
	glm::vec3 omega = glm::normalize(glm::vec3(0.0, 1.0, 0.0));
	float number_of_impulses_per_kernel = 56.0;
}

RayTracer::~RayTracer() {}

void RayTracer::init(const std::shared_ptr<Scene> scenePtr) {
}

void RayTracer::render(const std::shared_ptr<Scene> scenePtr) {
	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print("Start ray tracing at " + std::to_string(width) + "x" + std::to_string(height) + " resolution...");
	BVHisActive ? Console::print("BVH is active") : Console::print("BVH is not active");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear(scenePtr->backgroundColor());

	// <----  Preprocess scene ---->
	std::shared_ptr<Camera> camera = scenePtr->camera();
	glm::mat4 frameMatrix = inverse(camera->computeViewMatrix());

	// <---- Ray tracing code ---->
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			float x_clip = (static_cast<float>(x) / (width - 1)) * 2 - 1.0f;
			float y_clip = (static_cast<float>(y) / (height - 1)) * 2 - 1.0f;

			shared_ptr<Ray> ray = rayAt((float(x) + 0.5) / width, 1.f - (float(y) + 0.5) / height, frameMatrix, camera);

			shared_ptr<RayHit> rayHit = rayScene(ray, scenePtr);

			if (rayHit != nullptr) {
				m_imagePtr->operator()(x, y) = shade(scenePtr, rayHit, ray);
			}
			else {
				m_imagePtr->operator()(x, y) = scenePtr->backgroundColor();
			}
		}
	}

	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");

}

std::shared_ptr<Ray> RayTracer::rayAt(float x, float y, const glm::mat4& frameMatrix, const std::shared_ptr<Camera> camera) {
	glm::vec3 viewRight = normalize(glm::vec3(frameMatrix[0]));
	glm::vec3 viewUp = normalize(glm::vec3(frameMatrix[1]));
	glm::vec3 viewDir = -normalize(glm::vec3(frameMatrix[2]));
	glm::vec3 eye = glm::vec3(frameMatrix[3]);
	float w = 2.0 * float(tan(glm::radians(camera->getFoV() / 2.0)));
	glm::vec3 rayDir = glm::normalize(viewDir + ((x - 0.5f) * camera->getAspectRatio() * w) * viewRight + ((1.f - y) - 0.5f) * w * viewUp);
	return std::make_shared<Ray>(eye, rayDir);
}

std::shared_ptr<RayHit> RayTracer::rayScene(const std::shared_ptr<Ray>& ray, const std::shared_ptr<Scene> scenePtr) {
	std::shared_ptr<RayHit> nearestRayHit = nullptr;

	if (!BVHisActive) {
		for (int triangleIndex = 0; triangleIndex < scenePtr->numOfTriangles(); triangleIndex++) {
			const Triangle& triangle = scenePtr->triangle(triangleIndex);
			std::shared_ptr<RayHit> currentHit = ray->triangleIntersect(triangle.p0, triangle.p1, triangle.p2);

			if (currentHit) {
				if (!nearestRayHit || (nearestRayHit && currentHit->distance() < nearestRayHit->distance())) {
					nearestRayHit = currentHit;
					nearestRayHit->setTriangleData(triangleIndex);
				}
			}
		}
		return nearestRayHit;
	}

	return ray->intersectBVH(scenePtr->triangles(), scenePtr->sceneBVH());
}

glm::vec3 RayTracer::lightRadiance(const std::shared_ptr<LightSource>& lightPtr, const glm::vec3& position) const {
	return lightPtr->color() * lightPtr->intensity() * glm::pi<float>();
}

glm::vec3 RayTracer::materialReflectance(const std::shared_ptr<Scene> scenePtr,
	const std::shared_ptr<Material> materialPtr,
	const glm::vec3& wi,
	const glm::vec3& wo,
	const glm::vec2& fTextCoord,
	const glm::vec3& n) const {

	glm::vec3 albedo = materialPtr->hasAlbedoTexture() ?
		scenePtr->texture(materialPtr->getTextureBundle().m_albedoTexId)->fetch(fTextCoord) :
		materialPtr->albedo();

	float roughness = materialPtr->hasRoughnessTexture() ?
		scenePtr->texture(materialPtr->getTextureBundle().m_roughnessTexId)->fetch(fTextCoord).r :
		roughness = materialPtr->roughness();

	float metallicness = materialPtr->hasMetallicTexture() ?
		scenePtr->texture(materialPtr->getTextureBundle().m_metallicTexId)->fetch(fTextCoord).r :
		metallicness = materialPtr->metallicness();

	float ambiantOcclusion = materialPtr->hasAmbiantOcclusionTexture() ?
		scenePtr->texture(materialPtr->getTextureBundle().m_ambientOcclusionTexId)->fetch(fTextCoord).r :
		ambiantOcclusion = 1;

	return ambiantOcclusion * BRDF(wi, wo, n, albedo, roughness, metallicness);
}

glm::vec3 RayTracer::shade(const std::shared_ptr<Scene> scenePtr, const std::shared_ptr<RayHit>& rayHit, const std::shared_ptr<Ray> ray) {
	int triangleIndex = rayHit->triangleIndex();
	const Triangle& triangle = scenePtr->triangle(triangleIndex);

	std::shared_ptr<Model> currentModel = scenePtr->model(triangle.modelIndex);
	std::shared_ptr<Mesh> currentMesh = scenePtr->mesh(currentModel->meshId());
	const glm::mat4& invModelMatrix = glm::inverse(currentModel->transform().computeTransformMatrix());
	const glm::mat4& invNormalMatrix = glm::inverse(glm::transpose(invModelMatrix));
	const glm::vec3& center = currentMesh->getCenter();

	const glm::vec2& barycentricCoord = rayHit->uv_coord();
	float z = 1 - barycentricCoord.x - barycentricCoord.y;
	const glm::vec3 fPosition = z * triangle.p0 + barycentricCoord.x * triangle.p1 + barycentricCoord.y * triangle.p2;
	const glm::vec3 fNormal = z * triangle.n0 + barycentricCoord.x * triangle.n1 + barycentricCoord.y * triangle.n2;
	const glm::vec2 fTextCoord = z * triangle.uv0 + barycentricCoord.x * triangle.uv1 + barycentricCoord.y * triangle.uv2;

	const glm::vec3 localPos = glm::vec3(invModelMatrix * glm::vec4(fPosition, 1.0f));
	const glm::vec3 localNormal = glm::vec3(invNormalMatrix * glm::vec4(fNormal, 1.0f));

	glm::vec3 wo = glm::normalize(-ray->direction()); // normalize not necessary
	glm::vec3 n = normalize(fNormal);
	glm::vec3 res = glm::vec3(0, 0, 0);
	std::shared_ptr<Material> modelMaterial = scenePtr->material(triangle.materialIndex);


	for (int i = 0; i < scenePtr->numOfLights(); i++) {
		std::shared_ptr<LightSource> light = scenePtr->light(i);
		glm::vec3 info;
		if (light->type() == LightType::DirectionalLight) {
			info = light->forward();
		}
		else {
			info = light->attenuation();
		}

		glm::vec3 wi;
		float attenuation;
		if (light->type() == LightType::PointLight) {
			glm::vec3 lp = light->center() - fPosition;
			float d = length(lp);
			wi = normalize(lp);
			attenuation = 1 / (info.x + info.y * d + info.z * d * d);
			attenuation = 1 / (info.x + info.y * d + info.z * d * d);

		}
		else {
			wi = -normalize(info);
			attenuation = 1;
		}


		std::shared_ptr<RayHit> hitToLight = rayScene(std::make_shared<Ray>(fPosition + 0.01f * n + 0.15f * wi, wi), scenePtr);

		if (hitToLight) {
			continue;
		}

		float wiDotN = max(0.f, dot(wi, n));

		if (wiDotN <= 0.f)
			continue;
		modelMaterial->albedo(localPos, localNormal);
		modelMaterial->roughness(localPos, localNormal);
		modelMaterial->metallicness(localPos, localNormal);
		res += attenuation * lightRadiance(light, fPosition) * materialReflectance(scenePtr, modelMaterial, wi, wo, fTextCoord, n) * wiDotN;
	}
	return res;
}

