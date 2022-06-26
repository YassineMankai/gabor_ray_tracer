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

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"

class Mesh {
public:
	Mesh() = delete;
	Mesh(int id) :m_id(id) {};
	virtual ~Mesh ();
	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; } 
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; } 
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; } 
	inline const std::vector<glm::vec2> & vertexTexCoords() const { return m_vertexTexCoords; }
	inline std::vector<glm::vec2> & vertexTexCoords() { return m_vertexTexCoords; }
	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }
	inline const int& getId() const { return m_id; }
	inline const glm::vec3& getCenter() const { return m_center; }
	inline const float& getMeshScale() const { return m_meshScale; }

	/// Compute the parameters of a sphere which bounds the mesh
	void setBoundingSphere ();
	
	void recomputePerVertexNormals (bool angleBased = false);
	void computePlanarParameterizaton();
	void computeSphericalParameterizaton();

	void clear ();

private:
	int m_id; // For the moment, id = index in scene.m_meshes
	float m_meshScale = -1;
	glm::vec3 m_center;
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec2> m_vertexTexCoords;
	std::vector<glm::uvec3> m_triangleIndices;
};
