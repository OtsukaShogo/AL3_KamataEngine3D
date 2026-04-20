#pragma once
#include "KamataEngine.h"
#include"Collision.h"

class Player;

/// <summary>
/// 敵
/// </summary>
class Enemy {

public:
	Enemy();
	~Enemy();

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	//衝突応答
	void OnCollision(const Player* player);

	// === ゲッター ================================

	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//速度
	KamataEngine::Vector3 velocity_ = {};

	//経過時間
	float walkTimer_ = 0.0f;

	// === 定数 ============================================

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.025f;

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = 10.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = -15.0f;
	// アニメーション周期となる時間[秒]
	static inline const float kWalkMotionTime = 0.5f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
};
