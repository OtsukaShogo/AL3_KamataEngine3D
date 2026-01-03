#include "GameScene.h"
#include "AffineMatrix.h"

using namespace KamataEngine;

GameScene::GameScene() {};

GameScene::~GameScene() {
	delete modelPlayer;
	delete modelBlock_;
	delete player_;
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete debugCamera_;
	delete modelSkydome_;
}

void GameScene::Initialize() {
	//// テクスチャ読み込み
	//playerTextureHandle_ = TextureManager::Load("uvChecker.png");
	// 3Dモデルの生成
	modelPlayer = Model::CreateFromOBJ("player",true);
	modelBlock_ = Model::Create();

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// カメラの初期化
	camera_.farZ = 10000.0f;
	camera_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 自キャラの生成
	player_ = new Player();
	// 自キャラの初期化
	player_->Initialize(modelPlayer, &camera_);

	// 要素数
	const uint32_t kNumBlockVirtical = 10;
	const uint32_t kNumBlockHorizontal = 20;
	// ブロック1個分の幅
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;
	// 要素数を変更する
	worldTransformBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		// 一列の要素数を設定(縦方向のブロック数)
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// キューブの生成
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if ((i + j) % 2 == 1) {
				worldTransformBlocks_[i][j] = new WorldTransform();
				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
				worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
			}
		}
	}

	// === 天球 ========================================================================

	// 3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("fireFlyIV", true);
	// 生成
	skydome_ = new Skydome();
	// 初期化
	skydome_->Initialize(modelSkydome_, &camera_);
}

void GameScene::Update() {
	// 自キャラの更新
	player_->Update();

	// ブロックの更新
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			// アフィン変換行列の作成
			Matrix4x4 affineMatrix = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			worldTransformBlock->matWorld_ = affineMatrix;
			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}

	// 天球更新処理
	skydome_->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = true;
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		camera_.UpdateMatrix();
	}
}

void GameScene::Draw() {
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	skydome_->Draw();

	Model::PostDraw();
}