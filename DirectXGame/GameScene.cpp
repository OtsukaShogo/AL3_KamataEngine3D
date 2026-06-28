#include "GameScene.h"
#include "AffineMatrix.h"

using namespace KamataEngine;

GameScene::GameScene() {};

GameScene::~GameScene() {
	delete modelPlayer_;
	delete modelPlayerAttack_;
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

	delete fade_;
}

void GameScene::Initialize() {
	//// テクスチャ読み込み
	// playerTextureHandle_ = TextureManager::Load("uvChecker.png");
	//  3Dモデルの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelPlayerAttack_ = Model::CreateFromOBJ("hit_effect", true);
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
	player_->Initialize(modelPlayer_, modelPlayerAttack_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

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
	// 強制スクロール有効化
	cameraController_->SetMode(Mode::kForcedScroll);

	// フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);

	// フェードインフェーズから開始
	phase_ = Phase::kFadeIn;
}

void GameScene::Update() {

	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn: // フェードインフェーズ
		// フェード中もオブジェクトの行列を初期化するため通常と同じ更新を行う
		skydome_->Update();
		player_->Update();
		for (Enemy* newEnemy : enemies_) {
			newEnemy->Update();
		}
		cameraController_->Update();
		CameraUpdate();
		BlockUpdate();
		fade_->Update();
		break;

	case Phase::kPlay: // ゲームプレイフェーズ

		// 天球更新処理
		skydome_->Update();

		// 自キャラの更新
		player_->Update();

		// 雑魚敵の更新
		for (Enemy* newEnemy : enemies_) {
			newEnemy->Update();
		}

		// デスフラグの立った敵を削除
		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->GetIsDead_()) {
				delete enemy;
				return true;
			}
			return false;
		});

		// カメラコントローラ更新
		cameraController_->Update();

		// カメラの更新
		CameraUpdate();

		// ブロックの更新
		BlockUpdate();

		// === 当たり判定 ====================

		CheckAllCollisons();

		break;

	case Phase::kDeath: // デス演出フェーズ

		// 天球更新処理
		skydome_->Update();

		// 雑魚敵の更新
		for (Enemy* newEnemy : enemies_) {
			newEnemy->Update();
		}

		// デスパーティクルの更新
		if (deathParticles_) {
			deathParticles_->Update();
		}

		// カメラの更新
		CameraUpdate();

		// ブロックの更新
		BlockUpdate();

		break;

	case Phase::kFadeOut: // フェードアウトフェーズ
		fade_->Update();
		CameraUpdate();
		break;
	}
}

void GameScene::Draw() {
	Model::PreDraw();

	switch (phase_) {
	case Phase::kFadeIn: // フェードインフェーズ
	case Phase::kPlay:   // ゲームプレイフェーズ

		skydome_->Draw();

		BlockDraw();

		// 雑魚敵描画
		for (Enemy* newEnemy : enemies_) {
			newEnemy->Draw();
		}

		// 自キャラの描画
		player_->Draw();

		break;

	case Phase::kDeath:   // デス演出フェーズ
	case Phase::kFadeOut: // フェードアウトフェーズ

		skydome_->Draw();

		BlockDraw();

		// 雑魚敵描画
		for (Enemy* newEnemy : enemies_) {
			newEnemy->Draw();
		}

		if (deathParticles_) {
			deathParticles_->Draw();
		}

		break;
	}

	Model::PostDraw();

	// フェード描画（スプライトは3Dモデルの後）
	fade_->Draw();
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

		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵すべての当たり判定
		for (Enemy* enemy : enemies_) {
			if (enemy->IsCollisionDisabled()) {
				continue; // コリジョン無効の敵はスキップ
			}

			// 敵の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
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

void GameScene::CameraUpdate() {

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
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

void GameScene::BlockUpdate() {

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
}

void GameScene::BlockDraw() {
	// ブロックの描画
	for (std::vector<WorldTransform*> worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kFadeIn: // フェードインフェーズ
		// フェードイン完了でゲームプレイへ
		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}
		break;

	case Phase::kPlay: // ゲームプレイフェーズ
		if (player_->GetIsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlePosition = player_->GetWorldPosition();
			// デスパーティクルを発生・初期化
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticle_, &camera_, deathParticlePosition);
		}
		break;

	case Phase::kDeath: // デス演出フェーズ
		// デスパーティクル完了でフェードアウトへ
		if (deathParticles_ && deathParticles_->GetIsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		break;

	case Phase::kFadeOut: // フェードアウトフェーズ
		// フェードアウト完了でシーン終了
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}