// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <algorithm>
#include <limits>
#include <memory>
#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "BoundingBox.h"

        
class BVH {
    public:
        BVH() = delete;
        
        BVH(const std::vector<Triangle>& triangles, std::vector<int>& indices);
                
        inline virtual ~BVH () {}

        inline const std::shared_ptr<BVH>& left() { return m_left; }
        inline const std::shared_ptr<BVH>& right() { return m_right; }
        inline const BoundingBox& root() { return m_root; }
        std::vector<int> print();
        
    private:
        BoundingBox m_root;
        std::shared_ptr<BVH> m_left;
        std::shared_ptr<BVH> m_right;
};