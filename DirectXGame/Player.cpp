#define NOMINMAX
#include "Player.h"
#include "AffineMatrix.h"
#include "Easing.h"
#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;
using namespace MathUtility;

Player::Player() {};

Player::~Player() {}

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 攻撃エフェクトのワールド変換初期化
	worldTransformAttack_.Initialize();
	// 左右の自キャラ角度テーブル
	float destinationRotationYTable[] = {std::numbers::pi_v<float> * 3.0f / 2.0f, std ::numbers::pi_v<float> / 2.0f};

	// 状況に応じた角度を取得する
	worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
}

void Player::Update() {
	// 攻撃キーを押したら
	if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// 攻撃ビヘイビアをリクエスト
		behaviorRequest_ = Behavior::kAttack;
	}

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振るまいを変更する
		behavior_ = behaviorRequest_;
		// 各振るまいごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			// ルートビヘイビアの初期化
			BehaviorRootInitialize();

			break;

		case Behavior::kAttack:
			// 攻撃ビヘイビアの初期化
			BehaviorAttackInitialize();

			break;
		}
		// 振るまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	// 共通: 重力適用
	if (!onGround_) {
		velocity_.y += -kGravityAcceleration;
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	// マップ衝突（フラグを毎フレームリセット）
	CollisionMapInfo collisionMapInfo = {};

	switch (behavior_) {
		// 通常行動
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		collisionMapInfo.moveAmount = velocity_;
		break;

		// 攻撃行動
	case Behavior::kAttack:
		BehaviorAttackUpdate(collisionMapInfo);
		break;
	}
	CheckMapCollision(collisionMapInfo);
	isRightWallHit_ = collisionMapInfo.isRightWallHit;
	MoveAfterCollisionCheck(collisionMapInfo);
	CeilingHitMove(collisionMapInfo);
	ChengeGroundedState(collisionMapInfo);

	// 行列更新
	Matrix4x4 affineMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.matWorld_ = affineMatrix;
	worldTransform_.TransferMatrix();
}

void Player::MoveInput() {

	// 移動入力
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			KamataEngine::Vector3 acceleration = {};
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT)) {
				// 左移動中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenution);
				}
				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}

			} else if (KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 右移動中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenution);
				}
				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速/減速
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenution);
		}

		if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.y += kJumpAcceleration;
		}
	}
}

void Player::CheckMapCollision(CollisionMapInfo& info) {
	TopCheckMapCollision(info);
	BottomCheckMapCollision(info);
	RightCheckMapCollision(info);
	LeftCheckMapCollision(info);

	HitWall(info);
}

void Player::TopCheckMapCollision(CollisionMapInfo& info) {
	// 上昇あり?
	if (info.moveAmount.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionNew;

	KamataEngine::Vector3 pos = worldTransform_.translation_;
	pos.x += info.moveAmount.x;
	pos.y += info.moveAmount.y;
	pos.z += info.moveAmount.z;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(pos, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット?
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x, worldTransform_.translation_.y + kHeight / 2.0f, worldTransform_.translation_.z});
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先を排除する方向に移動量を設定する
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::max(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f + kBlank));
			// 天井に当たったことを記録する
			info.isCeilingHit = true;
		}
	}
}

void Player::BottomCheckMapCollision(CollisionMapInfo& info) {
	// 下降あり？
	if (info.moveAmount.y >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionNew;

	KamataEngine::Vector3 pos = worldTransform_.translation_;
	pos.x += info.moveAmount.x;
	pos.y += info.moveAmount.y;
	pos.z += info.moveAmount.z;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(pos, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x, worldTransform_.translation_.y - kHeight / 2.0f, worldTransform_.translation_.z});
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::min(0.0f, (rect.top - worldTransform_.translation_.y) + (kHeight / 2.0f + kBlank));
			// 地面に当たったことを記憶する
			info.isGrounded = true;
		}
	}
}

void Player::RightCheckMapCollision(CollisionMapInfo& info) {
	// 右移動あり
	if (info.moveAmount.x <= 0.0f) {
		return;
	}

	//  移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionNew;

	KamataEngine::Vector3 pos = worldTransform_.translation_;
	pos.x += info.moveAmount.x;
	pos.y += info.moveAmount.y;
	pos.z += info.moveAmount.z;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(pos, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x + kWidth / 2.0f, worldTransform_.translation_.y, worldTransform_.translation_.z});
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f + kBlank));
			info.isWallHit = true;
			info.isRightWallHit = true;
		}
	}
}

void Player::LeftCheckMapCollision(CollisionMapInfo& info) {
	// 左移動あり
	if (info.moveAmount.x >= 0.0f) {
		return;
	}

	//  移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionNew;

	KamataEngine::Vector3 pos = worldTransform_.translation_;
	pos.x += info.moveAmount.x;
	pos.y += info.moveAmount.y;
	pos.z += info.moveAmount.z;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(pos, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x - kWidth / 2.0f, worldTransform_.translation_.y, worldTransform_.translation_.z});
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::max(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f + kBlank));
			// 地面に当たったことを記憶する
			info.isWallHit = true;
		}
	}
}

void Player::CheckCameraSqueezeCollision() {
	KamataEngine::Vector3 rightTop = CornerPosition(worldTransform_.translation_, kRightTop);
	KamataEngine::Vector3 rightBottom = CornerPosition(worldTransform_.translation_, kRightBottom);

	MapChipField::IndexSet indexSet;

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightTop);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isDead_ = true;
		return;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		isDead_ = true;
	}
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}  //  kLeftTop
	};

	KamataEngine::Vector3 result = center;

	result.x += offsetTable[static_cast<uint32_t>(corner)].x;
	result.y += offsetTable[static_cast<uint32_t>(corner)].y;

	return result;
}

void Player::MoveAfterCollisionCheck(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_.x += info.moveAmount.x;
	worldTransform_.translation_.y += info.moveAmount.y;
	worldTransform_.translation_.z += info.moveAmount.z;
}

void Player::CeilingHitMove(const CollisionMapInfo& info) {
	// 天井に当たった
	if (info.isCeilingHit) {
		velocity_.y = 0.0f;
	}
}

void Player::ChengeGroundedState(const CollisionMapInfo& info) {
	// 自キャラが接地状態?
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 落下判定
			//   移動後の4つの角の座標
			std::array<KamataEngine::Vector3, kNumCorner> positionNew;

			KamataEngine::Vector3 pos = worldTransform_.translation_;
			pos.x += info.moveAmount.x;
			pos.y += info.moveAmount.y;
			pos.z += info.moveAmount.z;

			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(pos, static_cast<Corner>(i));
			}

			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;

			KamataEngine::Vector3 newPos;
			newPos.x = positionNew[kLeftBottom].x + 0.0f;
			newPos.y = positionNew[kLeftBottom].y + (-kBlank);
			newPos.z = positionNew[kLeftBottom].z + 0.0f;

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(newPos);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右下点の判定
			newPos.x = positionNew[kRightBottom].x + 0.0f;
			newPos.y = positionNew[kRightBottom].y + (-kBlank);
			newPos.z = positionNew[kRightBottom].z + 0.0f;

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(newPos);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {
				// 落下なら空中状態に切り替え
				onGround_ = false;
			}
		}

	} else {
		// 空中状態の処理
		// 着地フラグ
		if (info.isGrounded) {
			// 着地状態に切り替える(落下を止める)
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::HitWall(const CollisionMapInfo& info) {
	// 壁接触による減速
	if (info.isWallHit) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::Draw() {
	model_->Draw(worldTransform_, *camera_);

	// 攻撃中はエフェクトモデルを前方に描画
	if (behavior_ == Behavior::kAttack) {
		modelAttack_->Draw(worldTransformAttack_, *camera_);
	}
}

KamataEngine::Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;

	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() {
	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	if (IsAttack()) {
		return;//攻撃中はダメージ無効
	}

	(void)enemy;
	// デスフラグを立てる
	isDead_ = true;
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorRootUpdate() {
	MoveInput();

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> * 3.0f / 2.0f, std ::numbers::pi_v<float> / 2.0f};

		// 状況に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		worldTransform_.rotation_.y = (1.0f - EaseInOutQuad(t)) * worldTransform_.rotation_.y + EaseInOutQuad(t) * destinationRotationY;
	}
}

void Player::BehaviorAttackInitialize() {
	attackParameter_ = 0;
	attackPhase_ = AttackPhase::StartUp;
}

void Player::BehaviorAttackUpdate(CollisionMapInfo& info) {

	attackParameter_++;

	// 攻撃専用移動速度（velocity_ と分離してカメラ追従に影響させない）
	KamataEngine::Vector3 attackVelocity = {};

	switch (attackPhase_) {
		// 溜め動作
	case AttackPhase::StartUp:

	default: {
		float t = static_cast<float>(attackParameter_) / kAttackStartUpDuration;

		worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 1.6f, t);
		// 前進動作へ移行
		if (attackParameter_ >= kAttackStartUpDuration) {
			attackPhase_ = AttackPhase::Active;
			attackParameter_ = 0;
		}
		break;
	}

		// 突進動作
	case AttackPhase::Active: {
		float t = static_cast<float>(attackParameter_) / kAttackDuration;
		worldTransform_.scale_.z = EaseOut(0.3f, 1.3f, t);
		worldTransform_.scale_.y = EaseIn(1.6f, 0.7f, t);
		// 余韻動作へ移行
		if (attackParameter_ >= static_cast<uint32_t>(kAttackDuration)) {
			attackPhase_ = AttackPhase::Recovery;
			attackParameter_ = 0;
		}

		// 向いている方向に一定速度で自動移動
		if (lrDirection_ == LRDirection::kRight) {
			attackVelocity.x = kAttackMoveSpeed;
		} else {
			attackVelocity.x = -kAttackMoveSpeed;
		}

		break;
	}

		// 余韻動作
	case AttackPhase::Recovery: {
		float t = static_cast<float>(attackParameter_) / kAttackRecoveryDuration;
		worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

		// 既定の時間経過で攻撃終了して通常状態に戻す
		if (attackParameter_ >= kAttackRecoveryDuration) {
			behaviorRequest_ = Behavior::kRoot;
		}

		break;
	}
	}

	// velocity_（カメラ追従用）と attackVelocity（攻撃移動）を合算して moveAmount にセット
	info.moveAmount = {
	    velocity_.x + attackVelocity.x,
	    velocity_.y + attackVelocity.y,
	    velocity_.z + attackVelocity.z,
	};

	// 攻撃エフェクトをプレイヤーの前方に配置
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
	Matrix4x4 affineMatrixAttack = MakeAffineMatrix(worldTransformAttack_.scale_, worldTransformAttack_.rotation_, worldTransformAttack_.translation_);
	worldTransformAttack_.matWorld_ = affineMatrixAttack;
	worldTransformAttack_.TransferMatrix();
}

bool Player::IsAttack() const {
	if (behavior_ == Behavior::kAttack) {
		return true;
	}
	return false;
}