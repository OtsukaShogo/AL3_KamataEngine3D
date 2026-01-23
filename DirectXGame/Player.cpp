#define NOMINMAX
#include "Player.h"
#include "AffineMatrix.h"
#include "Easing.h"
#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <numbers>

Player::Player() {};

Player::~Player() {}

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 左右の自キャラ角度テーブル
	float destinationRotationYTable[] = {std::numbers::pi_v<float> * 3.0f / 2.0f, std ::numbers::pi_v<float> / 2.0f};

	// 状況に応じた角度を取得する
	worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
}

void Player::Update() {

	MoveInput();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;

	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity_;

	// マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	MoveAfterCollisionCheck(collisionMapInfo);

	CeilingHitMove(collisionMapInfo);

	ChengeGroundedState(collisionMapInfo);

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

	// アフィン変換行列の作成
	KamataEngine::Matrix4x4 affineMatrix = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.matWorld_ = affineMatrix;

	// 行列を定数バッファに転送
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
			velocity_.y = kJumpAcceleration;
		}

		// 空中
	} else {
		// 落下速度
		velocity_.y += -kGravityAcceleration;
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
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
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition({worldTransform_.translation_.x + kWidth/2.0f, worldTransform_.translation_.y, worldTransform_.translation_.z});
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = std::min(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f + kBlank));
			// 地面に当たったことを記憶する
			info.isWallHit = true;
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
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}
