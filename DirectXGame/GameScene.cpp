#include "GameScene.h"
#include "AffineMatrix.h"

using namespace KamataEngine;

GameScene::GameScene() {};

GameScene::~GameScene() {
	delete modelPlayer_;
	delete player_;

	delete deathParticles_;

	delete modelBlock_;
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete modelEnemy_;
	for (Enemy* newEnemy : enemies_) {
		delete newEnemy;
	}

	enemies_.clear();

	delete debugCamera_;

	delete modelSkydome_;

	delete mapChipField_;
}

void GameScene::Initialize() {
	//// テクスチャ読み込み
	// playerTextureHandle_ = TextureManager::Load("uvChecker.png");
	//  3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelBlock_ = Model::CreateFromOBJ("block", true);
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	modelDeathParticle_ = Model::CreateFromOBJ("deathParticle", true);

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// カメラの初期化
	camera_.farZ = 1200.0f;
	camera_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// === 天球 ========================================================================

	// 3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("fireFlyIV", true);
	// 生成
	skydome_ = new Skydome();
	// 初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// === マップチップ ===============================================

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	GenerateBlocks();

	// 自キャラの生成
	player_ = new Player();
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);
	// 自キャラの初期化
	player_->Initialize(modelPlayer_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	//仮の生成処理 後で消す
	deathParticles_ = new DeathParticles;
	deathParticles_->Initialize(modelDeathParticle_, &camera_, playerPosition);

	// 雑魚敵の生成
	for (int32_t i = 0; i < 3; ++i) {
		Enemy* newEnemy = new Enemy();
		// 座標をマップチップ番号で指定
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(30 + i, 18);
		// 雑魚敵の初期化
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);

		enemies_.push_back(newEnemy);
	}

	// カメラコントローラ
	cameraController_ = new CameraController();
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();
	// 範囲の指定
	cameraController_->SetMovableArea({10.0f, 100.0f, 5.0f, 20.0f});
}

void GameScene::Update() {

	if (KamataEngine::Input::GetInstance()->PushKey(DIK_R)) {
		Initialize();
	}

	// 自キャラの更新
	player_->Update();

	if (deathParticles_) {
		deathParticles_->Update();
	}

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

	// 雑魚敵の更新
	for (Enemy* newEnemy : enemies_) {
		newEnemy->Update();
	}

	// 天球更新処理
	skydome_->Update();

	// === 当たり判定 ===============================

	CheckAllCollisons();

	// カメラコントローラ更新
	cameraController_->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
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
		camera_.UpdateViewMatrix();
	}
}

void GameScene::Draw() {
	Model::PreDraw();

	// 自キャラの描画
	player_->Draw();

	if (deathParticles_) {
		deathParticles_->Draw();
	}


	// ブロックの描画
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 雑魚敵描画
	for (Enemy* newEnemy : enemies_) {
		newEnemy->Draw();
	}

	skydome_->Draw();

	Model::PostDraw();
}

void GameScene::GenerateBlocks() {
	// 要素数
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 配列を設定
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 一列の要素数を設定(縦方向のブロック数)
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// キューブの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::CheckAllCollisons() {
#pragma region 自キャラと敵キャラの当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1, aabb2;

		//自キャラの座標
		aabb1 = player_->GetAABB();

		//自キャラと敵すべての当たり判定
		for (Enemy* enemy : enemies_) {
		//敵の座標
			aabb2 = enemy->GetAABB();

			//AABB同士の交差判定
			if (CheckHitAABB(aabb1, aabb2)) {
				// 自キャラの衝突時交差判定
				player_->OnCollision(enemy);
				// 敵の衝突時関数を呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}