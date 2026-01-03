#pragma once
#include"KamataEngine.h"

struct Matrix3x3 {
	float m[3][3];
};

KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3 scale, KamataEngine::Vector3 rotate, KamataEngine::Vector3 translate);