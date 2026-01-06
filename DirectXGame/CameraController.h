#pragma once
#include"KamataEngine.h"

//前方宣言
class Player;

/// <summary>
/// カメラコントローラ
/// </summary>
class CameraController {

public:
	//矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	CameraController();
	~CameraController();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void Reset();

	void SetTarget(Player* target) { target_ = target; }

	void SetMovableArea(Rect area) { movableArea_ = area; }

	//座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	//速度掛け率
	static inline const float kVelocityBias = 20.0f;

	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {-10.0f, 15.0f, -10.0f, 15.0f};

private:

	//カメラ
	KamataEngine::Camera* camera_;

	Player* target_ = nullptr;

	//追従対象とカメラの座標の差(オフセット)
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};

	//カメラ移動範囲
	Rect movableArea_ = {0.0f, 100.0f, 0.0f, 100.0f};

	//カメラの目的座標
	KamataEngine::Vector3 targetPos_;
};
