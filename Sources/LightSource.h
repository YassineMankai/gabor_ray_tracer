#pragma once

#include "Transform.h"

enum class LightType { PointLight = 0, DirectionalLight = 1 };

class LightSource
{
public:
	inline LightSource(LightType type, glm::vec3 color, float intensity, glm::vec3 attenuation = glm::vec3(0.3,0.3,1)) :
		m_type(type), m_color(color), m_intensity(intensity), m_attenuation(attenuation) {};
	inline const Transform& transform() const { return m_transform; }
	inline Transform& transform() { return m_transform; }
	inline const glm::vec3 forward() const { return glm::vec3(m_transform.computeTransformMatrix() * glm::vec4(0, 0, 1, 0)) ; }
	inline const glm::vec3 center() const { return glm::vec3(m_transform.computeTransformMatrix() * glm::vec4(0, 0, 0, 1)) ; }
	inline const LightType type() const { return m_type; }
	inline const glm::vec3 color() const { return m_color; }
	inline  glm::vec3 color() { return m_color; }
	inline  glm::vec3 attenuation() { return m_attenuation; }
	inline const float intensity() const { return m_intensity; }
	inline  float intensity() { return m_intensity; }
private:
	LightType m_type;
	Transform m_transform;
	glm::vec3 m_color;
	glm::vec3 m_attenuation;
	float m_intensity;
};

