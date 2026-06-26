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
}

void TitleScene::Update() {

	//スペースが押されたらタイトルシーン終了
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		finished_ = true;
	}

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

	// タイトルのイージングアニメーション（往復）
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

	// タイトルY座標をイージングで更新
	worldTransformTitle_.translation_.y = kTitleBaseY + EaseInOutQuad(titleAnimTimer_) * kTitleAmplitude;

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
}
