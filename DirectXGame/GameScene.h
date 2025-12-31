#pragma once
#include "KamataEngine.h"
#include"Player.h"

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

private:

	//テクスチャハンドル
	uint32_t textureHandle_ = 0;
	
	//3Dモデル
	KamataEngine::Model* model_ = nullptr;

	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	//カメラ
	KamataEngine::Camera camera_;

	//自キャラ
	Player* player_ = nullptr;

};
