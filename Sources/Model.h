#pragma once

#include <memory>
#include <vector>
#include "Mesh.h"
#include "Material.h"

class Model
{
public:
	inline const Transform& transform() const { return m_transform; }
	inline Transform& transform() { return m_transform; }
	inline void useMesh(int meshId) { m_mesh_index = meshId; }
	inline void useMaterial(int materialId) { m_material_index = materialId; }
	inline const int meshId() const { return m_mesh_index; }
	inline const int materialId() const { return m_material_index; }
private:
	Transform m_transform;
	int m_mesh_index;
	int m_material_index;
};

