#define NOMINMAX
#include "Enemy.h"
#include "Easing.h"
#include "WorldTransformUpdate.h"
#include <numbers>
#include "Player.h"

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
	// ビヘイビアリクエストの処理
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;
		switch (behavior_) {
		case Behavior::kWalk:
			BehaviorWalkInitialize();
			break;
		case Behavior::kDeath:
			BehaviorDeathInitialize();
			break;
		default:
			break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}

	// ビヘイビアごとの更新
	switch (behavior_) {
	case Behavior::kWalk:
		BehaviorWalkUpdate();
		break;
	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;
	default:
		break;
	}

	// 共通: 行列の更新
	WorldTransformUpdate(worldTransform_);
}

void Enemy::BehaviorWalkInitialize() {
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer_ = 0.0f;
}

void Enemy::BehaviorWalkUpdate() {
	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// タイマーを更新
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin((2.0f * std::numbers::pi_v<float>) * (walkTimer_ / kWalkMotionTime));
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = (std::numbers::pi_v<float> / 180.0f) * degree;
}

void Enemy::BehaviorDeathInitialize() {
	deathTimer_ = 0.0f;
}

void Enemy::BehaviorDeathUpdate() {
	// タイマーを加算
	deathTimer_ += 1.0f / 60.0f;

	float t = std::min(deathTimer_ / kDeathDuration, 1.0f);

	// Y軸まわりの回転をイージングで変化
	worldTransform_.rotation_.y = EaseOut(std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 2.0f, t);

	// X軸まわりの回転をイージングで変化（前のめりに倒れる）
	worldTransform_.rotation_.x = EaseIn(0.0f, std::numbers::pi_v<float> / 2.0f, t);

	// 一定時間に達したらデスフラグを立てる
	if (deathTimer_ >= kDeathDuration) {
		isDead_ = true;
	}
}

void Enemy::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;

	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {
	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
	if (behavior_ == Behavior::kDeath) {
		// 敵がやられているなら何もしない
		return;
	}

	// プレイヤーが攻撃中なら敵が死ぬ
	if (player->IsAttack()) {
		// 敵の振るまいをデス演出に変更
		behaviorRequest_ = Behavior::kDeath;

		// コリジョン無効フラグを立てる
		isCollisionDisabled_ = true;
	}
}