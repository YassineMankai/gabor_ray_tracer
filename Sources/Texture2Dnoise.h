/*
    adapted from the code provided with the "Procedural Noise using Sparse Gabor Convolution" article
*/


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

#include "Texture.h"



float gabor(float K, float a, float F_0, float omega_0, float x, float y);
unsigned int morton(unsigned int x, unsigned int y);

class prng
{
public:
    void seed(unsigned int s) { m_seed = s; gen.seed(s); }
    float uniform(float min, float max) 
    {
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }
    unsigned int poisson(float mean) 
    {
        std::poisson_distribution<> d(mean);
        return d(gen);
    }
private:
    unsigned int m_seed;
    std::default_random_engine gen;
};

class Texture2Dnoise
{
public:
    Texture2Dnoise(bool isIsotropic, float K, float a, float F_0, float omega_0, float number_of_impulses_per_kernel, unsigned random_offset)
        : m_isIsotropic(isIsotropic), m_magnitude(K), m_kernel_freq_width(a), m_frequency(F_0), m_orientation(omega_0), m_random_offset(random_offset)
    {
        m_kernel_radius = std::sqrt(-std::log(0.05) / M_PI) / m_kernel_freq_width;
        m_impulse_density = number_of_impulses_per_kernel / (M_PI * m_kernel_radius * m_kernel_radius);
    }
    float cell(int i, int j, float x, float y) const;
    float variance() const;
    std::shared_ptr<Texture> generateColor2DNoiseTexture(int currentIndex, int resolution, const std::vector<glm::vec3>& colorMap_);
    std::shared_ptr<Texture> generateFloat2DNoiseTexture(int currentIndex, int resolution);

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
