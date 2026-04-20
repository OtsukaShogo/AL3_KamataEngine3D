#include "Skydome.h"
#include"AffineMatrix.h"

Skydome::Skydome() {}

Skydome::~Skydome() {}

void Skydome::Initialize(KamataEngine::Model* model,KamataEngine::Camera* camera) {

	assert(model);

	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.scale_ = {500.0f, 500.0f, 500.0f};
}

void Skydome::Update() {
	// アフィン変換行列の作成
	KamataEngine::Matrix4x4 affineMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.matWorld_ = affineMatrix;

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Skydome::Draw() {
	//3Dモデルの描画
	model_->Draw(worldTransform_, *camera_);
}