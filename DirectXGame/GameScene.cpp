#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene() {};

GameScene::~GameScene() { 
	delete model_;
	delete player_;
}

void GameScene::Initialize() { 
	//テクスチャ読み込み
	textureHandle_ = TextureManager::Load("uvChecker.png");
	//3Dモデルの生成
	model_ = Model::Create();

	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//カメラの初期化
	camera_.Initialize();

	//自キャラの生成
	player_ = new Player();
	//自キャラの初期化
	player_->Initialize(model_, textureHandle_, &camera_);
}

void GameScene::Update() {
	//自キャラの更新
	player_->Update();
}

void GameScene::Draw() { 
	Model::PreDraw();

	//自キャラの描画
	player_->Draw();

	Model::PostDraw();
}