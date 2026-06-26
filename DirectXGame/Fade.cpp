#include "Fade.h"

#include <algorithm>

Fade::Fade() {}

Fade::~Fade() {}

void Fade::Initialize() {
	textureHandle_ = KamataEngine::TextureManager::Load("white1x1.png");
	sprite_ = KamataEngine::Sprite::Create(textureHandle_, KamataEngine::Vector2(0, 0));
	sprite_->SetSize(KamataEngine::Vector2(1280, 720));
	sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, 1));
}

void Fade::Update() {
	switch (status_) {
	case Status::None:
		// 何もしない
		break;

	case Status::FadeIn:
		// === フェードイン処理 ============================================================

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 1.0fから0.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を小さくする
		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));
		break;

	case Status::FadeOut:
		// === フェードアウト処理 ============================================================

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	KamataEngine::Sprite::PreDraw();
	sprite_->Draw();
	KamataEngine::Sprite::PostDraw();
}

// フェード開始
void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
	// 開始時の色を即座に設定（1フレーム目のちらつき防止）
	if (status_ == Status::FadeIn) {
		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, 1)); // 黒から始まる
	} else if (status_ == Status::FadeOut) {
		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, 0)); // 透明から始まる
	}
}

// フェード停止
void Fade::Stop() { status_ = Status::None; }

//フェード終了判定
bool Fade::IsFinished() const {
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}