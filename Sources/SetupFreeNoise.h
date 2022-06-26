#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <climits>
#include <iomanip>
#include <memory>
#include <random>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

#include "Material.h"
#include "Texture.h"

class SetupFreeNoise
{
public:
    SetupFreeNoise(bool isIsotropic, float K, float a, float F_0, float omega_0, float number_of_impulses_per_kernel, int random_offset)
        : m_isIsotropic(isIsotropic), m_magnitude(K), m_kernel_freq_width(a), m_frequency(F_0), m_orientation(omega_0), m_random_offset(random_offset)
    {
        m_kernel_radius = std::sqrt(-std::log(0.05) / M_PI) / m_kernel_freq_width;
        m_impulse_density = number_of_impulses_per_kernel / (2 * M_PI * m_kernel_radius * m_kernel_radius * m_kernel_radius);
    }
    float cell(int i, int j, int k, const glm::vec3 &fracPos, const glm::vec3& normal) const;
    float noiseFloat(const glm::vec3& pos, const glm::vec3& normal);
    glm::vec3 noiseColor(const glm::vec3& pos, const glm::vec3& normal, const std::vector<glm::vec3>& colorMap);
    float variance() const;

private:
    bool m_isIsotropic;
    float m_magnitude;
    float m_kernel_freq_width;
    float m_frequency;
    float m_orientation;
    float m_kernel_radius;
    float m_impulse_density;
    unsigned m_random_offset;
};

class SurfaceNoiseMaterial : public Material {
public:
    SurfaceNoiseMaterial(int id, glm::vec3 albedo, float roughness, float metallicness, const TextureBundle& textureBundle = { -1,-1, -1, -1 })
        :Material(id, albedo, roughness, metallicness, textureBundle) {};
    void setGen(std::shared_ptr<SetupFreeNoise> gen);
    void addColor(const std::vector<glm::vec3>& colors);
    virtual glm::vec3& albedo(const glm::vec3& pos, const glm::vec3& normal) override;
    virtual float& metallicness(const glm::vec3& pos, const glm::vec3& normal) override;
    virtual float& roughness(const glm::vec3& pos, const glm::vec3& normal) override;
    void setmask(bool activateAlbedoNoise, bool activateRoughnessNoise, bool activateMetalicNoise);
private:
    std::shared_ptr< SetupFreeNoise> m_generator;
    std::vector<glm::vec3> m_colorMap;
    bool noiseActive[3] = { 1,0,0 }; // i0:albedo  i1:roughness i2:metalic
};