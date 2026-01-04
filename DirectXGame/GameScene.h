#pragma once
#include "KamataEngine.h"
#include"Player.h"
#include<vector>
#include"Skydome.h"
#include"MapChipField.h"

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
	
	//3Dモデル
	KamataEngine::Model* modelPlayer = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;

	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	//カメラ
	KamataEngine::Camera camera_;

	//自キャラ
	Player* player_ = nullptr;

	//ブロック
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

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
};
