#include"AffineMatrix.h"

KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3 scale, KamataEngine::Vector3 rotate, KamataEngine::Vector3 translate) {
	Matrix3x3 rotateMatrix;
	rotateMatrix.m[0][0] = cosf(rotate.y) * cosf(rotate.z);
	rotateMatrix.m[0][1] = -(cosf(rotate.y))*sinf(rotate.z);
	rotateMatrix.m[0][2] = sinf(rotate.y);

	rotateMatrix.m[1][0] = cosf(rotate.x) * sinf(rotate.z) + sinf(rotate.x) * sinf(rotate.y) * cosf(rotate.z);
	rotateMatrix.m[1][1] = cosf(rotate.x) * cosf(rotate.z) - sinf(rotate.x) * sinf(rotate.y) * sinf(rotate.z);
	rotateMatrix.m[1][2] = -(sinf(rotate.x))*cosf(rotate.y);

	rotateMatrix.m[2][0] = sinf(rotate.x) * sinf(rotate.z) - cosf(rotate.x) * sinf(rotate.y) * cosf(rotate.z);
	rotateMatrix.m[2][1] = sinf(rotate.x) * cosf(rotate.z) + cosf(rotate.x) * sinf(rotate.y) * sinf(rotate.z);
	rotateMatrix.m[2][2] = cosf(rotate.x) * cosf(rotate.y);

	KamataEngine::Matrix4x4 result;
	result.m[0][0] = scale.x * rotateMatrix.m[0][0];
	result.m[0][1] = scale.x * rotateMatrix.m[0][1];
	result.m[0][2] = scale.x * rotateMatrix.m[0][2];
	result.m[0][3] = 0.0f;

	result.m[1][0] = scale.y * rotateMatrix.m[1][0];
	result.m[1][1] = scale.y * rotateMatrix.m[1][1];
	result.m[1][2] = scale.y * rotateMatrix.m[1][2];
	result.m[1][3] = 0.0f;

	result.m[2][0] = scale.z * rotateMatrix.m[2][0];
	result.m[2][1] = scale.z * rotateMatrix.m[2][1];
	result.m[2][2] = scale.z * rotateMatrix.m[2][2];
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeRotateZMatrix(float angle) {
	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = std::cos(angle);
	result.m[0][1] = -std::sin(angle);
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = std::sin(angle);
	result.m[1][1] = std::cos(angle);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Vector3 Transform(const KamataEngine::Vector3& v, const KamataEngine::Matrix4x4& m) {
	KamataEngine::Vector3 result{};

	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];

	return result;
}
