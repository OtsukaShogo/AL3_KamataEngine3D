#pragma once
#include "KamataEngine.h"
#include"Collision.h"

class Player;

/// <summary>
/// 敵
/// </summary>
class Enemy {

public:
	// ふるまい
	enum class Behavior {
		kWalk,  // 歩行
		kDeath, // 死

		kUnknown // 不明
	};

	Enemy();
	~Enemy();

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	//衝突応答
	void OnCollision(const Player* player);

	//状態管理
	void BehaviorWalkInitialize();
	void BehaviorWalkUpdate();

	void BehaviorDeathInitialize();
	void BehaviorDeathUpdate();

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	// === ゲッター ================================

	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();

	// デスフラグ
	bool GetIsDead_() const { return isDead_; }

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

	// デス演出タイマー
	float deathTimer_ = 0.0f;

	// デスフラグ
	bool isDead_ = false;

	//ふるまい
	Behavior behavior_ = Behavior::kWalk;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	//無効フラグ
	bool isCollisionDisabled_ = false;

	// === 定数 ============================================

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.025f;

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = 10.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = -15.0f;
	// アニメーション周期となる時間[秒]
	static inline const float kWalkMotionTime = 0.5f;

	// デス演出時間[秒]
	static inline const float kDeathDuration = 0.5f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
};
