#include "DeathParticles.h"
#include "WorldTransformUpdate.h"
#include"AffineMatrix.h"
#include <algorithm>

DeathParticles::DeathParticles() {}

DeathParticles::~DeathParticles() {}

void DeathParticles::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}

	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
}

void DeathParticles::Update() {

	//終了なら何もしない
	if (isFinished_) {
		return;
	}

	for (uint32_t i = 0; i < 8; ++i) {
	//基本となる速度ベクトル
		KamataEngine::Vector3 velocity = {kSpeed, 0.0f, 0.0f};
		//回転角を計算する
		float angle = kAngleUnit * i;
		//Z軸まわり回転行列
		KamataEngine::Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);
		//基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);

		//移動処理
		worldTransforms_[i].translation_.x += velocity.x;
		worldTransforms_[i].translation_.y += velocity.y;
		worldTransforms_[i].translation_.z += velocity.z;
	}

	//カウンターを1フレーム分の数秒進める
	counter_ += 1.0f / 60.0f;

	//存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		//終了扱いにする
		isFinished_ = true;
	}

	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);
	//色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	// 行列の更新
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		WorldTransformUpdate(worldTransform);
	}
}

void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		// 3Dモデルを描画
		model_->Draw(worldTransform, *camera_,&objectColor_);
	}
}