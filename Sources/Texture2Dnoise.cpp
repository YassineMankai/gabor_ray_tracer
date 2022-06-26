#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "Texture2DNoise.h"

float gabor(float K, float a, float F_0, float omega_0, float x, float y)
{
    float gaussian_envelop = K * std::exp(-M_PI * (a * a) * ((x * x) + (y * y)));
    float sinusoidal_carrier = std::cos(2.0 * M_PI * F_0 * ((x * std::cos(omega_0 * M_PI)) + (y * std::sin(omega_0 * M_PI))));
    return gaussian_envelop * sinusoidal_carrier;
}

unsigned int morton(unsigned int x, unsigned int y)
{
    unsigned int z = 0;
    for (unsigned int i = 0; i < (sizeof(unsigned int) * CHAR_BIT); ++i) {
        z |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
    }
    return z;
}


glm::vec2 randomFreqOrient(prng &gen, const float &frequency) {
    float test = gen.uniform(0.0, 2.0);
    float orientation;
    if (test < 1.0)
        orientation = gen.uniform(-0.2, +0.2);
    else
        orientation = gen.uniform(-0.9, -0.5);

    return glm::vec2(gen.uniform(0.2f * frequency, 0.8f * frequency), orientation);
}


float Texture2Dnoise::cell(int i, int j, float x, float y) const
{
    unsigned s = morton(i, j) + m_random_offset; // nonperiodic noise
    if (s == 0) s = 1;
    prng gen;
    gen.seed(s);
    double number_of_impulses_per_cell = m_impulse_density * m_kernel_radius * m_kernel_radius;
    unsigned number_of_impulses = gen.poisson(number_of_impulses_per_cell);
    float noise = 0.0;
    for (unsigned i = 0; i < number_of_impulses; ++i) {
        float x_i = gen.uniform(0, 1);
        float y_i = gen.uniform(0, 1);
        float w_i = gen.uniform(-1.0, +1.0);
        float x_i_x = x - x_i;
        float y_i_y = y - y_i;
        if (((x_i_x * x_i_x) + (y_i_y * y_i_y)) < 1.0) {
            if (m_isIsotropic)
                noise += w_i * gabor(m_magnitude, m_kernel_freq_width, m_frequency, gen.uniform(-1, +1), x_i_x * m_kernel_radius, y_i_y * m_kernel_radius);
            else {
                noise += w_i * gabor(m_magnitude, m_kernel_freq_width, m_frequency, m_orientation, x_i_x * m_kernel_radius, y_i_y * m_kernel_radius);
            }
        }
    }
    return noise;
}

float Texture2Dnoise::variance() const
{
    float integral_gabor_filter_squared = ((m_magnitude * m_magnitude) / (4.0 * m_kernel_freq_width * m_kernel_freq_width)) * (1.0 + std::exp(-(2.0 * M_PI * m_frequency * m_frequency) / (m_kernel_freq_width * m_kernel_freq_width)));
    return m_impulse_density * (1.0 / 3.0) * integral_gabor_filter_squared;
}

std::shared_ptr<Texture> Texture2Dnoise::generateColor2DNoiseTexture(int currentIndex, int resolution, const std::vector<glm::vec3>& colorMap_) {
    float* image = new float[resolution * resolution * 3];
    float scale = 3.0f * std::sqrt(variance());
    for (unsigned row = 0; row < resolution; ++row) {
        for (unsigned col = 0; col < resolution; ++col) {
            float x = (float(col) + 0.5) - (float(resolution) / 2.0);
            float y = (float(resolution - row - 1) + 0.5) - (float(resolution) / 2.0);
            x /= m_kernel_radius, y /= m_kernel_radius;
            float int_x = std::floor(x), int_y = std::floor(y);
            float frac_x = x - int_x, frac_y = y - int_y;
            int i = int(int_x), j = int(int_y);
            float noise = 0.0;
            for (int di = -1; di <= +1; ++di) {
                for (int dj = -1; dj <= +1; ++dj) {
                    noise += cell(i + di, j + dj, frac_x - di, frac_y - dj);
                }
            }
            noise = std::max(0.0f, std::min(0.5f + (0.5f * (noise / scale)), 1.1f))  * colorMap_.size();
            int interval = static_cast<int>(noise);
            float frac = noise - interval;
            glm::vec3 color;
            if (interval >= colorMap_.size() - 1) {
                color = colorMap_[colorMap_.size() - 1];
            }
            else {
                float frac = noise - floor(noise);
                color = (1 - frac) * colorMap_[interval] + frac * colorMap_[interval + 1];
            }
            int pixelIndex = (row * resolution) + col;
            image[3 * pixelIndex] = color.x;
            image[3 * pixelIndex + 1] = color.y;
            image[3 * pixelIndex + 2] = color.z;
        }
    }
    return std::make_shared<Texture>(currentIndex, resolution, resolution, 3, image, true);
}

std::shared_ptr<Texture> Texture2Dnoise::generateFloat2DNoiseTexture(int currentIndex, int resolution) {
    float* image = new float[resolution * resolution];
    float scale = 2.5f * std::sqrt(variance());
    for (unsigned row = 0; row < resolution; ++row) {
        for (unsigned col = 0; col < resolution; ++col) {
            float x = (float(col) + 0.5) - (float(resolution) / 2.0);
            float y = (float(resolution - row - 1) + 0.5) - (float(resolution) / 2.0);
            x /= m_kernel_radius, y /= m_kernel_radius;
            float int_x = std::floor(x), int_y = std::floor(y);
            float frac_x = x - int_x, frac_y = y - int_y;
            int i = int(int_x), j = int(int_y);
            float noise = 0.0;
            for (int di = -1; di <= +1; ++di) {
                for (int dj = -1; dj <= +1; ++dj) {
                    noise += cell(i + di, j + dj, frac_x - di, frac_y - dj);
                }
            }
            noise = std::max(0.0f, std::min(0.5f + (0.5f * (noise / scale)), 1.1f));
            int pixelIndex = (row * resolution) + col;
            image[pixelIndex] = noise;
        }
    }
    return std::make_shared<Texture>(currentIndex, resolution, resolution, 1, image, true);
}