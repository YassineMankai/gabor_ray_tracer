// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once


#include <cmath>
#include <algorithm>
#include <memory>
#include <iostream>
#include <limits>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "BoundingBox.h"
#include "BVH.h"



class RayHit {
public:
	RayHit(glm::vec2 coordinates, float rayCoordinate, float triangleIndex = -1) :uv_coordinates(coordinates), m_distance(rayCoordinate), m_triangleIndex(triangleIndex){ };
	virtual ~RayHit() {};
	inline const glm::vec2& uv_coord() const { return uv_coordinates; }
	inline const float& distance() const { return m_distance; }
	inline const int& triangleIndex() const { return m_triangleIndex; }
	inline void setTriangleData(const int& triangleIndex) {
		m_triangleIndex = triangleIndex;
	}

private:
	glm::vec2 uv_coordinates;
	float m_distance;
	int m_triangleIndex;
};

class Ray {
public:

	Ray(const glm::vec3& origin, const glm::vec3& direction)
		:m_origin(origin), m_direction(direction) {};
	virtual ~Ray() {};
	inline const glm::vec3& origin() const { return m_origin; }
	inline const glm::vec3& direction() const { return m_direction; }
	
	std::shared_ptr<RayHit> triangleIntersect(const glm::vec3& p0,
		const glm::vec3& p1,
		const glm::vec3& p2) const;

	bool boxIntersect(const BoundingBox& box, float& nearT, float& farT) const;
	
	std::shared_ptr<RayHit> intersectBVH(const std::vector<Triangle>& triangles, const std::shared_ptr<BVH>& bvh) const;

private:
	glm::vec3 m_origin;
	glm::vec3 m_direction;
};