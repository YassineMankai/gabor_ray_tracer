// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Ray.h"

using namespace std;

std::shared_ptr<RayHit> Ray::triangleIntersect(
	const glm::vec3& p0,
	const glm::vec3& p1,
	const glm::vec3& p2) const {
	const glm::vec3 e0 = p1 - p0;
	const glm::vec3 e1 = p2 - p0;
	const glm::vec3 n = normalize(cross(e0, e1));
	const glm::vec3 q = cross(m_direction, e1);
	float a = dot(e0, q);

	if ((dot(n, m_direction) >= 0) || (abs(a) < 0.00000001f)) {
		return nullptr;
	}

	const  glm::vec3 s = (m_origin - p0) / a;
	const  glm::vec3 r = cross(s, e0);
	float b0 = dot(s, q);
	float b1 = dot(r, m_direction);
	float b2 = 1 - b0 - b1;
	if ((b0 < 0) || (b1 < 0) || (b2 < 0)) {
		return nullptr;
	}

	float t = dot(e1, r);

	if (t >= 0) {
		return std::make_shared<RayHit>(glm::vec2(b0, b1), t);
	}
	return nullptr;

};


bool Ray::boxIntersect(
	const BoundingBox& box,
	float& nearT,
	float& farT) const {
	nearT = std::numeric_limits<float>::min();
	farT = std::numeric_limits<float>::max();
	glm::vec3 dRcp(1.f / m_direction[0], 1.f / m_direction[1], 1.f / m_direction[2]);
	for (int i = 0; i < 3; i++) {
		const float direction = m_direction[i];
		const float origin = m_origin[i];
		const float minVal = box.min()[i], maxVal = box.max()[i];
		if (direction == 0) {
			if (origin < minVal || origin > maxVal)
				return false;
		}
		else {
			float t1 = (minVal - origin) * dRcp[i];
			float t2 = (maxVal - origin) * dRcp[i];
			if (t1 > t2)
				std::swap(t1, t2);
			nearT = std::max(t1, nearT);
			farT = std::min(t2, farT);
			if (!(nearT <= farT))
				return false;
		}
	}
	return true;
}

std::shared_ptr<RayHit> Ray::intersectBVH(const std::vector<Triangle>& triangles, const std::shared_ptr<BVH>& bvh) const {
	
	if (bvh == nullptr)
		return nullptr;

	const BoundingBox& box = bvh->root();
	float a, b;

	if (!boxIntersect(box, a, b))
		return nullptr;

	if (bvh->left() == nullptr && bvh->right() == nullptr)
	{
		int trIndex = box.triangleIndex();
		const Triangle triangle = triangles[trIndex];
		std::shared_ptr<RayHit> hit = triangleIntersect(triangle.p0, triangle.p1, triangle.p2);
		if (hit) {
			hit->setTriangleData(trIndex);
		}	
		return hit;
	}
	
	std::shared_ptr<RayHit> hitLeft = intersectBVH(triangles, bvh->left());
	std::shared_ptr<RayHit> hitRight = intersectBVH(triangles, bvh->right());
	
	if (!hitLeft && !hitRight)
		return nullptr;
	
	if (hitRight && !hitLeft)
		return hitRight;

	if (hitLeft && !hitRight)
		return hitLeft;

	float distLeft = hitLeft->distance();
	float distRight = hitRight->distance();

	return (distLeft < distRight) ? hitLeft : hitRight;

}