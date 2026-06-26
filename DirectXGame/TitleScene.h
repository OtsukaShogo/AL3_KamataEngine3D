#pragma once
#include "KamataEngine.h"

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene {

public:
	TitleScene();
	~TitleScene();

	void Initialize();
	void Update();
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

private:
	// カメラ
	KamataEngine::Camera camera_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// プレイヤーモデル
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::WorldTransform worldTransformPlayer_;

	// タイトルモデル
	KamataEngine::Model* modelTitle_ = nullptr;
	KamataEngine::WorldTransform worldTransformTitle_;

	// タイトルY座標アニメーション
	float titleAnimTimer_ = 0.0f;
	bool titleAnimForward_ = true;

	// タイトル基準Y座標（ワールド座標）
	static inline const float kTitleBaseY = 3.0f;
	// 上下振幅
	static inline const float kTitleAmplitude = 0.5f;
	// 半周期（秒）
	static inline const float kTitleHalfPeriod = 1.0f;

	// 終了フラグ
	bool finished_ = false;
};
