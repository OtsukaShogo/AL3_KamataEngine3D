#include "Enemy.h"
#include "WorldTransformUpdate.h"
#include <numbers>

Enemy::Enemy() {}

Enemy::~Enemy() {}

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 角度
	worldTransform_.rotation_.y = std ::numbers::pi_v<float> / 2.0f;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// タイマーを更新
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin((2.0f * std::numbers::pi_v<float>)*(walkTimer_ / kWalkMotionTime));
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = (std::numbers::pi_v<float> / 180.0f) * degree;

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}

void Enemy::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}