#include "KamataEngine.h"
#include <Windows.h>
#include"TitleScene.h"
#include"GameScene.h"

using namespace KamataEngine;

// シーン（型）
enum class Scene {

	kUnknown = 0,

	kTitle,
	kGame,
};

// 現在シーン（型）
Scene scene = Scene::kUnknown;

void ChangeScene(TitleScene*& titleScene, GameScene*& gameScene);

void UpdateScene(TitleScene* titleScene, GameScene* gameScene);

void DrawScene(TitleScene* titleScene, GameScene* gameScene);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"AL3");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//最初のシーンを生成
	scene = Scene::kTitle;
	TitleScene* titleScene = nullptr;
	titleScene = new TitleScene();
	titleScene->Initialize();

	//ゲームシーンのインスタンス生成
	GameScene* gameScene = nullptr;

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		//シーン切り替え
		ChangeScene(titleScene, gameScene);

		// シーン更新
		UpdateScene(titleScene, gameScene);

		//描画開始
		dxCommon->PreDraw();

		//シーン描画
		DrawScene(titleScene, gameScene);

		//描画終了
		dxCommon->PostDraw();
	}

	KamataEngine::Finalize();

	//ゲームシーンの解放
	delete titleScene;
	delete gameScene;

	return 0;
}

void ChangeScene(TitleScene*& titleScene, GameScene*& gameScene) {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene(TitleScene* titleScene, GameScene* gameScene) {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

void DrawScene(TitleScene* titleScene, GameScene* gameScene) {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}