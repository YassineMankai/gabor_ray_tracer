// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::setBoundingSphere () {
	m_center = glm::vec3 (0.0);
	m_meshScale = 0.f;
	for (const auto & p : m_vertexPositions)
		m_center += p;
	m_center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		m_meshScale = std::max (m_meshScale, distance (m_center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) {
	m_vertexNormals.clear ();
	// Change the following code to compute a proper per-vertex normal
	m_vertexNormals.resize (m_vertexPositions.size (), glm::vec3 (0.0, 0.0, 0.0));
	for (auto & t : m_triangleIndices) {
		glm::vec3 e0 (m_vertexPositions[t[1]] - m_vertexPositions[t[0]]);
		glm::vec3 e1 (m_vertexPositions[t[2]] - m_vertexPositions[t[0]]);
		glm::vec3 n = normalize (cross (e0, e1));
		for (size_t i = 0; i < 3; i++)
			m_vertexNormals[t[i]] += n;
	}
	for (auto & n : m_vertexNormals)
		n = normalize (n);
}

void Mesh::computePlanarParameterizaton() {
	setBoundingSphere();
	m_vertexTexCoords.clear();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	for (auto& v : m_vertexPositions) {
		minX = std::min(minX, v.x);
		maxX = std::max(maxX, v.x);
		minY = std::min(minY, v.y);
		maxY = std::max(maxY, v.y);
	}

	for (auto& v : m_vertexPositions) {
		m_vertexTexCoords.push_back(glm::vec2((v.x - minX) / (maxX - minX), (v.y - minY) / (maxY - minY)));
	}
}

void Mesh::computeSphericalParameterizaton() {
	setBoundingSphere();
	m_vertexTexCoords.clear();
	for (auto& v : m_vertexPositions) {
		glm::vec3 normalizedPos = glm::normalize(v - m_center);
		float theta = acos(normalizedPos.z);
		float phi;
		if (abs(normalizedPos.x) > 1e-12f || abs(normalizedPos.y) > 1e-12f) {
			phi = atan2(normalizedPos.y, normalizedPos.x);
		}
		else
		{
			phi = 0;
		}
		m_vertexTexCoords.push_back(glm::vec2(theta / M_PI, (phi + M_PI) / (2 * M_PI)));
	}
}

void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_triangleIndices.clear ();
}