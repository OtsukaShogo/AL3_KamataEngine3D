#pragma once
#include "KamataEngine.h"

class Player {

public:
	Player();
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenution = 0.2f;
	static inline const float kLimitRunSpeed = 5.0f;
	//旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	//重力加速度
	static inline const float kGravityAcceleration = 9.8f;
	//最大落下速度
	static inline const float kLimitFallSpeed = 20.0f;
	//ジャンプ初速
	static inline const float kJumpAcceleration = 1.0f;

	enum class LRDirection { kRight, kLeft };

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	//接地状態フラグ
	bool onGround_ = true;
};
