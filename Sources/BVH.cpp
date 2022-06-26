// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "BVH.h"
 
float calculateMedian(const std::vector<Triangle>& triangles, const std::vector<int>& indices, int axis) {  // O(log(n) can be better)
	std::vector<float> v;
	
	for (int i = 0; i < indices.size(); i++) {
		v.push_back((triangles[indices[i]].p0[axis] + triangles[indices[i]].p1[axis] + triangles[indices[i]].p2[axis]) / 3);
	}
	std::sort(v.begin(), v.end());
	
	if (v.size() % 2 == 1)
		return v[v.size() / 2];
	else
		return v[v.size() / 2 - 1];
}


BVH::BVH(const std::vector<Triangle>& triangles, std::vector<int>& indices) {
	if (indices.size() == 0) {
		for (int i = 0; i < triangles.size(); i++) {
			indices.push_back(i);
		}
	}	

	m_root.init(triangles[indices[0]].p0);
	for (int i = 0; i < indices.size(); i++) {
		m_root.extendTo(triangles[indices[i]].p0);
		m_root.extendTo(triangles[indices[i]].p1);
		m_root.extendTo(triangles[indices[i]].p2);
	}
		
	if (indices.size() == 1) {
		m_root.setTriangleIndex(indices[0]);
	}
	else
	{
		int axis = m_root.dominantAxis();
		float median = calculateMedian(triangles, indices, axis);
		
		std::vector<int> leftIndices;
		std::vector<int> rightIndices;
		for (int i = 0; i < indices.size(); i++) {		
			float com = (triangles[indices[i]].p0[axis] + triangles[indices[i]].p1[axis] + triangles[indices[i]].p2[axis]) / 3;
			
			if (com <= median) {
				leftIndices.push_back(indices[i]);
			}
			else {
				rightIndices.push_back(indices[i]);
			}
		}

		if (rightIndices.size() == 0) {
			int s = leftIndices.size();
			if (s == 1) {
				m_left = std::make_shared<BVH>(triangles, leftIndices);
				return;
			}
			else 
			{
				for (int i = 0; i < s / 2; i++) {
					rightIndices.push_back(leftIndices.back());
					leftIndices.pop_back();
				}
			}
		}
		else if (leftIndices.size() == 0) {
			int s = rightIndices.size();
			if (s == 1) {
				m_right = std::make_shared<BVH>(triangles, rightIndices);
				return;
			}
			else
			{
				for (int i = 0; i < s / 2; i++) {
					leftIndices.push_back(rightIndices.back());
					leftIndices.pop_back();
				}
			}
		}

		m_left = std::make_shared<BVH>(triangles, leftIndices);
		m_right = std::make_shared<BVH>(triangles, rightIndices);
	}
}

std::vector<int> BVH::print() {
	std::vector<int> res;
	if (!m_left && !m_right)
		res.push_back(m_root.triangleIndex());
	if (m_left) {
		std::vector<int> l = m_left->print();
		res.insert(res.end(), l.begin(), l.end());
	}
	if (m_right) {
		std::vector<int> r = m_right->print();
		res.insert(res.end(), r.begin(), r.end());
	}
	sort(res.begin(), res.end());
	return res;
}