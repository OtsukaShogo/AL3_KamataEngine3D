#pragma once
#include "KamataEngine.h"

/// <summary>
/// フェード
/// </summary>
class Fade {
public:
	enum class Status {
		None,    // 待機中
		FadeIn,  // フェードイン（黒→透明）
		FadeOut, // フェードアウト（透明→黒）
	};

	Fade();
	~Fade();

	void Initialize();
	void Update();
	void Draw();

	// フェード開始
	void Start(Status status,float duration);

	//フェード停止
	void Stop();

	//フェード終了判定
	bool IsFinished() const;

	Status GetStatus() const { return status_; }

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	uint32_t textureHandle_ = 0;

	Status status_ = Status::None;
	float alpha_ = 0.0f;
	float duration_ = 1.0f;
	float counter_ = 0.0f;
};
