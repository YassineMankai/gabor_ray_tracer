#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "Solid3DNoise.h"
#include "Texture2Dnoise.h"

float gabor3D(float K, float a, float F_0, glm::vec3 orientation, glm::vec3 pos)
{
    float gaussian_envelop = K * std::exp(-M_PI * (a * a) * ((pos.x * pos.x) + (pos.y * pos.y) + (pos.z * pos.z)));
    float sinusoidal_carrier = std::cos(2.0 * M_PI * F_0 * glm::dot(pos, orientation));
    return gaussian_envelop * sinusoidal_carrier;
}

float Solid3DNoise::cell(int i, int j, int k, const glm::vec3 &pos) const
{
    unsigned s = m_random_offset + i + j * 400 + (400 * 400 * k);
    if (s == 0) s = 1;
    prng gen;
    gen.seed(s);
    double number_of_impulses_per_cell = m_impulse_density * m_kernel_radius * m_kernel_radius * m_kernel_radius;
    unsigned number_of_impulses = gen.poisson(number_of_impulses_per_cell);
    float noise = 0.0;
    for (int i = 0; i < number_of_impulses; ++i) {  
        glm::vec3 samplePos = glm::vec3(gen.uniform(0, 1), gen.uniform(0, 1), gen.uniform(0, 1)); 
        glm::vec3 fracPos = pos - samplePos;
        float w_i = gen.uniform(-1.0f, 1.0f);
        if (fracPos.x * fracPos.x + fracPos.y + fracPos.z * fracPos.z < 1.0f) { // inside cylindre
            if (m_isIsotropic)
                noise += w_i * gabor3D(m_magnitude, m_kernel_freq_width, m_frequency, glm::normalize(glm::vec3(gen.uniform(-1, 1), gen.uniform(-1, 1), gen.uniform(-1, 1))), fracPos * m_kernel_radius); // isotropic
            else {
                noise += w_i * gabor3D(m_magnitude, m_kernel_freq_width, m_frequency, m_orientation, fracPos * m_kernel_radius); // anisotropic  
            }
        }
    } 
    
    return noise;
}

float Solid3DNoise::variance() const
{
    float integral_gabor_filter_squared = ((m_magnitude * m_magnitude) / (4.0 * m_kernel_freq_width * m_kernel_freq_width)) * (1.0 + std::exp(-(2.0 * M_PI * m_frequency * m_frequency) / (m_kernel_freq_width * m_kernel_freq_width * m_kernel_freq_width)));
    return m_impulse_density * (1.0 / 3.0) * integral_gabor_filter_squared;
}

glm::vec3 Solid3DNoise::noiseColor(const glm::vec3& pos, const std::vector<glm::vec3>& colorMap) {
    glm::vec3 fracPos = 180.0f * pos / m_kernel_radius;
    int i = int(fracPos.x), j = int(fracPos.y), k = int(fracPos.z);
    fracPos = fracPos - floor(fracPos);
    float value = 0.0;
    for (int di = -1; di <= +1; ++di) {
        for (int dj = -1; dj <= +1; ++dj) {
            for (int dk = -1; dk <= +1; ++dk) {
                value += cell(i + di, j + dj, k + dk, fracPos - glm::vec3(di, dj, dk));
            }
        }
    }
    float noiseEval = std::max(0.00f, std::min(1.01f, 0.4f + value / (60 * variance()))) * colorMap.size();
    int interval = static_cast<int>(noiseEval);
    float frac = noiseEval - interval;
    glm::vec3 color;
    if (interval >= colorMap.size() -1 )
        color = colorMap[colorMap.size() - 1];
    else
        color = (1 - frac) * colorMap[interval] + frac * colorMap[interval + 1];
    return color;
}

float Solid3DNoise::noiseFloat(const glm::vec3& pos) {
    glm::vec3 fracPos = 180.0f * pos / m_kernel_radius;
    int i = int(fracPos.x), j = int(fracPos.y), k = int(fracPos.z);
    fracPos = fracPos - floor(fracPos);
    float value = 0.0;
    for (int di = -1; di <= +1; ++di) {
        for (int dj = -1; dj <= +1; ++dj) {
            for (int dk = -1; dk <= +1; ++dk) {
                value += cell(i + di, j + dj, k + dk, fracPos - glm::vec3(di, dj, dk));
            }
        }
    }
    return std::max(0.00f, std::min(1.01f, 0.4f + value / (60 * variance())));
}

void SolidNoiseMaterial::addColor(const std::vector<glm::vec3> & colors) {
    m_colorMap.insert(m_colorMap.end(), colors.begin(), colors.end());
}

void SolidNoiseMaterial::setGen(std::shared_ptr<Solid3DNoise> gen)
{
    m_generator = gen;
}

glm::vec3& SolidNoiseMaterial::albedo(const glm::vec3& pos, const glm::vec3& normal) {
    if (noiseActive[0]) {
        m_albedo = m_generator->noiseColor(pos, m_colorMap);
    }
    return m_albedo;
}

float& SolidNoiseMaterial::roughness(const glm::vec3& pos, const glm::vec3& normal) {
    
    if (noiseActive[1]) {
        m_roughness = 0.05 + 1.1 * m_generator->noiseFloat(pos);
    }
    return m_roughness;
}

float& SolidNoiseMaterial::metallicness(const glm::vec3& pos, const glm::vec3& normal) {
    if (noiseActive[2]) {
        m_metallicness = 0.05 + 0.7 * m_generator->noiseFloat(pos);
    }
    return m_metallicness;
}

void SolidNoiseMaterial::setmask(bool activateAlbedoNoise, bool activateRoughnessNoise, bool activateMetalicNoise) {
    noiseActive[0] = activateAlbedoNoise;
    noiseActive[1] = activateRoughnessNoise;
    noiseActive[2] = activateMetalicNoise;
}