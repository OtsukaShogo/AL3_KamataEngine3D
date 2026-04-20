#pragma once
#include"KamataEngine.h"

struct AABB {
	KamataEngine::Vector3 min{};
	KamataEngine::Vector3 max{};
};

bool CheckHitAABB(AABB aabb1, AABB aabb2);