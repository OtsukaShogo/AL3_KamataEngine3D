#pragma once
#include "KamataEngine.h"
#include<array>
#include <numbers>

/// <summary>
// デス演出用パーティクル
/// </summary>
class DeathParticles {

public:
	DeathParticles();
	~DeathParticles();

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

	bool GetIsFinished() const { return isFinished_; }

	// === 定数 ========================================

	//パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	// 存続時間
	static inline const float kDuration = 1.0f;

	// 移動の速さ
	static inline const float kSpeed = 0.1f;

	// 分裂した1個分の角度
	static inline const float kAngleUnit = std::numbers::pi_v<float> * 2.0f / 8.0f;

private:
	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//終了フラグ
	bool isFinished_ = false;

	//経過時間カウント
	float counter_ = 0.0f;

	KamataEngine::ObjectColor objectColor_;

	KamataEngine::Vector4 color_;
};
