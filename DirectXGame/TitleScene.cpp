#include "TitleScene.h"
#include "Easing.h"
#include "WorldTransformUpdate.h"
#include <numbers>

using namespace KamataEngine;

TitleScene::TitleScene() {}

TitleScene::~TitleScene() {
	delete modelPlayer_;
	delete modelTitle_;
	delete debugCamera_;
	delete fade_;
}

void TitleScene::Initialize() {
	// カメラ初期化
	camera_.Initialize();
	debugCamera_ = new DebugCamera(1280, 720);

	// プレイヤーモデル
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.translation_ = {0.0f, -5.0f, 0.0f};
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float>;
	worldTransformPlayer_.scale_ = {5.0f, 5.0f, 5.0f};

	// タイトルモデル
	modelTitle_ = Model::CreateFromOBJ("titleFont", true);
	worldTransformTitle_.Initialize();
	worldTransformTitle_.translation_ = {0.0f, kTitleBaseY, 0.0f};
	worldTransformTitle_.scale_ = {2.0f, 2.0f, 2.0f};

	titleAnimTimer_ = 0.0f;
	titleAnimForward_ = true;

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);
}

void TitleScene::Update() {

	// デバッグカメラ切り替え
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		// フェードイン完了でメインへ
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		// タイトルのイージングアニメーション（往復）
		{
			float delta = 1.0f / 60.0f / kTitleHalfPeriod;
			if (titleAnimForward_) {
				titleAnimTimer_ += delta;
				if (titleAnimTimer_ >= 1.0f) {
					titleAnimTimer_ = 1.0f;
					titleAnimForward_ = false;
				}
			} else {
				titleAnimTimer_ -= delta;
				if (titleAnimTimer_ <= 0.0f) {
					titleAnimTimer_ = 0.0f;
					titleAnimForward_ = true;
				}
			}
			worldTransformTitle_.translation_.y = kTitleBaseY + EaseInOutQuad(titleAnimTimer_) * kTitleAmplitude;
		}
		// スペースでフェードアウト開始
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		// フェードアウト完了でシーン終了
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	// ワールド行列更新
	WorldTransformUpdate(worldTransformPlayer_);
	WorldTransformUpdate(worldTransformTitle_);
}

void TitleScene::Draw() {
	// 3Dモデル描画
	Model::PreDraw();
	modelPlayer_->Draw(worldTransformPlayer_, camera_);
	modelTitle_->Draw(worldTransformTitle_, camera_);
	Model::PostDraw();

	// フェード描画（スプライトは3Dモデルの後）
	fade_->Draw();
}
