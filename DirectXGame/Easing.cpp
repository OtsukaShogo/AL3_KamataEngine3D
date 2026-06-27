#include "Easing.h"

float EaseInOutQuad(float t) {
	if (t < 0.5f) {
		return 2.0f * t * t;
	} else {
		return 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
	}
}

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

float EaseOut(float from, float to, float t) {
	float eased = 1.0f - (1.0f - t) * (1.0f - t);
	return from + (to - from) * eased;
}

float EaseIn(float from, float to, float t) {
	float eased = t * t;
	return from + (to - from) * eased;
}