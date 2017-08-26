#include "color_process.h"
#include <cmath>
#include <cassert>

template<typename T>
T mix(T l, T r, T m) {
	return (r - l) * m + l;
}

template<typename T>
T mix(T l, T r, bool m) {
	return mix(l, r, m ? 1.0f : 0.f);
}

float encode_impl(float a) {
	a *= a;
	return (1 - a) / (a + 1);
}

float decode_impl(float a) {
	return sqrt((1 - a) / (1 + a));
}

template <typename T, int dims>
class Vec {
public:
	constexpr static const int size = sizeof(T) * dims;
	T& operator[](int idx)
	{
		assert(idx < dims && idx >= 0);
		return _val[idx];
	}
private:
	T _val[dims];
};

class Mat {
public:
	Mat(void* ptr, int width, int height) : _ptr((char*)ptr), cols(width), rows(height) {}
	template<typename T>
	T& at(int y, int x) {
		auto offset = y * T::size * cols + x * T::size;
		return reinterpret_cast<T*>(_ptr + offset)[0];
	}
	int rows;
	int cols;
private:
	char* _ptr;
};

float clip(float f) {
	if (f < 0) {
		return 0;
	}
	if (f > 1) {
		return 1;
	}
	return f;
}

using Vec4b = Vec<unsigned char, 4>;

namespace blhx {

void encode(void* _img, int width, int height) {
	Mat img(_img, width, height);
	for (auto i = 0; i<img.rows; i++) {
		for (auto j = 0; j<img.cols; j++) {
			auto& color = img.at<Vec4b>(i, j);
			if (color[3] == 0) {
				color[0] = color[1] = color[2] = 255;
				continue;
			}
			auto r = color[0] / 255.f;
			auto g = color[1] / 255.f;
			auto b = color[2] / 255.f;
			auto x = mix(0.2f, 0.8f, b >= 0.25);
			auto y = mix(0.2f, 0.5f, b >= 0.75);
			r = mix(r, r * (1.0f - x) + (0.5f * x), r >= 0.5f);
			g = mix(g, g * (1.0f - y) + (0.5f * y), g >= 0.5f);
			color[0] = encode_impl(r) * 255.f;
			color[1] = encode_impl(g) * 255.f;
			color[2] = encode_impl(b) * 255.f;
		}
	}
}

void decode(void* _img, int width, int height) {
	Mat img(_img, width, height);
	for (auto i = 0; i<img.rows; i++) {
		for (auto j = 0; j<img.cols; j++) {
			auto& color = img.at<Vec4b>(i, j);
			auto r = decode_impl(color[0] / 255.f);
			auto g = decode_impl(color[1] / 255.f);
			auto b = decode_impl(color[2] / 255.f);
			auto x = mix(0.2f, 0.8f, b >= 0.25);
			auto y = mix(0.2f, 0.5f, b >= 0.75);
			r = mix(r, (r - 0.5f * x) * (1.0f / (1.0f - x)), r >= 0.5f);
			g = mix(g, (g - 0.5f * y) * (1.0f / (1.0f - y)), g >= 0.5f);
			color[0] = clip(r) * 255.f;
			color[1] = clip(g) * 255.f;
			color[2] = clip(b) * 255.f;
		}
	}
}

}