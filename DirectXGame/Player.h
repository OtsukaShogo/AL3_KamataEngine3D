#pragma once
#include "Collision.h"
#include "KamataEngine.h"

class MapChipField;

class Enemy;

class Player {

public:
	enum class LRDirection { kRight, kLeft };

	// マップとの当たり判定
	struct CollisionMapInfo {
		bool isCeilingHit = false;
		bool isGrounded = false;
		bool isWallHit = false;
		bool isRightWallHit = false;
		KamataEngine::Vector3 moveAmount;
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	// ふるまい
	enum class Behavior {
		kRoot,   // 通常状態
		kAttack, // 攻撃中

		kUnknown // 不明
	};

	// 攻撃フェーズ
	enum class AttackPhase {
		StartUp, // 溜め
		Active,  // 突進
		Recovery // 余韻
	};

	Player();
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Enemy* enemy);

	//攻撃しているか
	bool IsAttack() const;

	// === ゲッター ==============================

	KamataEngine::WorldTransform const& GetWorldTransform() const { return worldTransform_; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();

	// デスフラグ
	bool GetIsDead() const { return isDead_; }

	// === セッター ==============================

	void SetMapChipField(MapChipField* mapChipField) { this->mapChipField_ = mapChipField; }

	void SetPositionX(float x) { worldTransform_.translation_.x = x; }

	// カメラ押し出し後に呼ぶ。右側にブロックがあれば挟まれ死亡
	void CheckCameraSqueezeCollision();

private:
	void MoveInput();

	/// <summary>
	/// マップチップとの当たり判定
	/// </summary>
	/// <param name="info"></param>
	void CheckMapCollision(CollisionMapInfo& info);

	void TopCheckMapCollision(CollisionMapInfo& info);
	void BottomCheckMapCollision(CollisionMapInfo& info);
	void RightCheckMapCollision(CollisionMapInfo& info);
	void LeftCheckMapCollision(CollisionMapInfo& info);

	/// <summary>
	/// 当たっていた場合の処理
	/// </summary>
	/// <param name="info"></param>
	void MoveAfterCollisionCheck(const CollisionMapInfo& info);

	void CeilingHitMove(const CollisionMapInfo& info);

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	/// <summary>
	/// 接地状態の切り替え
	/// </summary>
	/// <param name="info"></param>
	void ChengeGroundedState(const CollisionMapInfo& info);

	void HitWall(const CollisionMapInfo& info);

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate(CollisionMapInfo& info);

private:
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// 攻撃用モデル
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::WorldTransform worldTransformAttack_;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// CollisionMapInfo collisionMapInfo_;

	KamataEngine::Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 接地状態フラグ
	bool onGround_ = true;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// デスフラグ
	bool isDead_ = false;

	// 右壁衝突フラグ
	bool isRightWallHit_ = false;

	// ふるまい
	Behavior behavior_ = Behavior::kRoot;
	// 次のふるまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 攻撃ギミックの経過時間カウンター
	uint32_t attackParameter_ = 0;

	// 現在の攻撃フェーズ
	AttackPhase attackPhase_;

	// === 定数 ================================================

	static inline const float kAcceleration = 0.02f;
	static inline const float kAttenution = 0.2f;
	static inline const float kLimitRunSpeed = 0.3f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	// 重力加速度
	static inline const float kGravityAcceleration = 2.0f / 60.0f;
	// 最大落下速度
	static inline const float kLimitFallSpeed = 100.0f;
	// ジャンプ初速
	static inline const float kJumpAcceleration = 27.0f / 60.0f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kBlank = 0.1f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.1f;

	////接地判定を下げる微小な数値
	// static inline const float kGroundCheckOffset = 0.01f;

	// 衝突時の速度減衰率
	static inline const float kAttenuationWall = 0.1f;
	// 攻撃中の移動速度
	static inline const float kAttackMoveSpeed = 0.3f;
	// 攻撃時間
	static inline const float kAttackStartUpDuration = 5.0f;
	static inline const float kAttackDuration = 5.0f;
	static inline const float kAttackRecoveryDuration = 5.0f;
};
