#pragma once

float EaseInOutQuad(float t);

float Lerp(float a, float b, float t);

// from→to を二次イーズアウト（速→遅）で補間
float EaseOut(float from, float to, float t);

// from→to を二次イーズイン（遅→速）で補間
float EaseIn(float from, float to, float t);