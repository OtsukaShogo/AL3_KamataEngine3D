#pragma once
#include "KamataEngine.h"
#include"Player.h"
#include<vector>
#include"Skydome.h"
#include"MapChipField.h"
#include"CameraController.h"
#include"Enemy.h"

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

	void GenerateBlocks();

private:

	////テクスチャハンドル
	//uint32_t playerTextureHandle_ = 0;

	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	//カメラ
	KamataEngine::Camera camera_;

	//自キャラ
	Player* player_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelPlayer_= nullptr;

	//ブロック
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	// 3Dモデル
	KamataEngine::Model* modelBlock_ = nullptr;

	//雑魚敵
	Enemy* enemy_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelEnemy_ = nullptr;

	//デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	//デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	//天球
	Skydome* skydome_ = nullptr;
	//3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	//マップチップフィールド
	MapChipField* mapChipField_;

	//カメラコントローラ
	CameraController* cameraController_;
};
