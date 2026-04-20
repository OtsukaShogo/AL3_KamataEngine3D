#define NOMINMAX
#include "CameraController.h"
#include"Player.h"
#include<algorithm>
#include"Easing.h"

CameraController::CameraController() {}

CameraController::~CameraController() {}

void CameraController::Initialize(KamataEngine::Camera* camera) {
	// カメラの初期化
	camera_ = camera;
}

void CameraController::Update() {
	//追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetPos_.x = targetWorldTransform.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias;
	targetPos_.y = targetWorldTransform.translation_.y + targetOffset_.y + target_->GetVelocity().y * kVelocityBias;
	targetPos_.z = targetWorldTransform.translation_.z + targetOffset_.z + target_->GetVelocity().z * kVelocityBias;

	//座標補間によりゆったり追従
	camera_->translation_.x = Lerp(camera_->translation_.x, targetPos_.x, kInterpolationRate);
	camera_->translation_.y = Lerp(camera_->translation_.y, targetPos_.y, kInterpolationRate);
	camera_->translation_.z = Lerp(camera_->translation_.z, targetPos_.z, kInterpolationRate);

	//追跡対象が画面外に出ないように補正
	camera_->translation_.x = std::max(camera_->translation_.x, targetWorldTransform.translation_.x + margin.left);
	camera_->translation_.x = std::min(camera_->translation_.x, targetWorldTransform.translation_.x + margin.right);
	camera_->translation_.y = std::max(camera_->translation_.y, targetWorldTransform.translation_.y + margin.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, targetWorldTransform.translation_.y + margin.top);

	//移動範囲制限
	camera_->translation_.x = std::max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = std::min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = std::max(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = std::min(camera_->translation_.y, movableArea_.top);

	//行列を更新する
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	//追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの座標を計算
	camera_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	camera_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	camera_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
}