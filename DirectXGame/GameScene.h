#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include"Fade.h"
#include <vector>

// ゲームのフェーズ
enum class Phase {
	kFadeIn, // フェードイン
	kPlay,   // ゲームプレイ
	kDeath,  // デス演出
	kFadeOut // フェードアウト
};

// ゲームシーン
class GameScene {

public:
	GameScene();
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

private:
	void GenerateBlocks();

	// すべての当たり判定を行う
	void CheckAllCollisons();

	//カメラの更新
	void CameraUpdate();

	//ブロックの更新
	void BlockUpdate();

	//ブロックの描画
	void BlockDraw();

	//フェーズ切り替え
	void ChangePhase();

private:
	////テクスチャハンドル
	// uint32_t playerTextureHandle_ = 0;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// 自キャラ
	Player* player_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	// デス演出用パーティクル
	DeathParticles* deathParticles_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelDeathParticle_;

	// ブロック
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	// 3Dモデル
	KamataEngine::Model* modelBlock_ = nullptr;

	// 雑魚敵
	std::list<Enemy*> enemies_;
	// 3Dモデル
	KamataEngine::Model* modelEnemy_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;

	// カメラコントローラ
	CameraController* cameraController_;

	// ゲームの現在のフェーズ
	Phase phase_;

	// フェード
	Fade* fade_ = nullptr;
	// フェード時間（秒）
	static inline const float kFadeDuration = 1.0f;

	// 終了フラグ
	bool finished_ = false;
};
