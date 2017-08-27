#include "color_process.h"
#include <cmath>
#include <cassert>
#include "AssetsTools/TextureFileFormat.h"

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

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
void decompressBlockAlphaC(uint8* data, uint8* img, int width, int height, int ix, int iy, int channels);
void decompressBlockETC2c(unsigned int block_part1, unsigned int block_part2, uint8 *img, int width, int height, int startx, int starty, int channels);

// read color block from data stream
void readColorBlockETC(byte **stream, unsigned int &block1, unsigned int &block2)
{
	byte *data = *stream;
	block1 = 0;           block1 |= data[0];
	block1 = block1 << 8; block1 |= data[1];
	block1 = block1 << 8; block1 |= data[2];
	block1 = block1 << 8; block1 |= data[3];
	block2 = 0;           block2 |= data[4];
	block2 = block2 << 8; block2 |= data[5];
	block2 = block2 << 8; block2 |= data[6];
	block2 = block2 << 8; block2 |= data[7];
	*stream = data + 8;
}

void ETC2RGB4ToRGBA32(void *in, void* out, int width, int height) {
	byte *data, *stream, rgba[4 * 4 * 4], *lb;
	unsigned int block1, block2;
	int x, y, w, h, bpp;
	int lx, ly, lw, lh;

	// init
	w = width;
	h = height;
	bpp = 4;
	data = (byte*)out;
	stream = (byte*)in;

	for (y = 0; y < h; y += 4)
	{
		for (x = 0; x < w; x += 4)
		{
			readColorBlockETC(&stream, block1, block2);
			decompressBlockETC2c(block1, block2, rgba, 4, 4, 0, 0, 4);
			lb = rgba;
			lh = min(y + 4, h) - y;
			lw = min(x + 4, w) - x;
			for (ly = 0; ly < lh; ly++, lb += 4 * 4)
				for (lx = 0; lx < lw; lx++) {
					memcpy(data + (w*(y + ly) + x + lx)*bpp, lb + lx * 4, 3);
					data[(w*(y + ly) + x + lx)*bpp + 3] = 255;
				}
		}
	}
}

void ETC2RGBA8ToRGBA32(void *in, void* out, int width, int height) {
	byte *data, *stream, rgba[4 * 4 * 4], *lb;
	unsigned int block1, block2;
	int x, y, w, h, bpp;
	int lx, ly, lw, lh;

	// init
	w = width;
	h = height;
	bpp = 4;
	data = (byte*)out;
	stream = (byte*)in;

	for (y = 0; y < h; y += 4)
	{
		for (x = 0; x < w; x += 4)
		{
			// EAC block + ETC2 RGB block
			decompressBlockAlphaC(stream, rgba + 3, 4, 4, 0, 0, 4);
			stream += 8;
			readColorBlockETC(&stream, block1, block2);
			decompressBlockETC2c(block1, block2, rgba, 4, 4, 0, 0, 4);
			lb = rgba;
			lh = min(y + 4, h) - y;
			lw = min(x + 4, w) - x;
			for (ly = 0; ly < lh; ly++, lb += 4 * 4)
				for (lx = 0; lx < lw; lx++)
					memcpy(data + (w*(y + ly) + x + lx)*bpp, lb + lx * 4, 4);
		}
	}
}

void RGBA2ARGB(void *in, void*out, size_t size) {
	auto buf = static_cast<unsigned int*>(in), ret = static_cast<unsigned int*>(out);
	for (auto i = 0; i<size; i++) {
		ret[i] = ((buf[i] << 8) & 0xffffff00) | ((buf[i] >> 24) & 0xff);
	}
}

unsigned char rgb4_lut[16] = { 0, 16, 32, 51, 68, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 255 };

void RGBA42RGBA(void *in, void*out, size_t size) {
	auto buf = static_cast<unsigned char*>(in);
	auto ret = static_cast<unsigned char*>(out);

	for (auto i = 0; i<size * 2; i++) {
		ret[i * 2] = rgb4_lut[buf[i] & 0xf];
		ret[i * 2 + 1] = rgb4_lut[buf[i] >> 4];
	}
}

void ARGB2RGBA(void *in, void*out, size_t size) {
	auto buf = static_cast<unsigned int*>(in), ret = static_cast<unsigned int*>(out);
	for (auto i = 0; i<size; i++) {
		ret[i] = ((buf[i] >> 8) & 0xffffff) | ((buf[i] & 0xff) << 24);
	}
}

void* convertTextureFormat(void* buf, int width, int height, bool& needFree, TextureFormat from, TextureFormat to) {
	needFree = false;

	if (from == to) {
		return buf;
	}

	void* ret = NULL;

	if (to == TexFmt_RGBA32) {
		if (from == TexFmt_ARGB32) {
			auto size = width * height * 4;
			ret = malloc(width * height * 4);
			needFree = true;

			ARGB2RGBA(buf, ret, width * height);
		}
		else if (from == TexFmt_ETC2_RGBA8) {
			auto size = width * height * 4;
			ret = malloc(width * height * 4);
			needFree = true;

			ETC2RGBA8ToRGBA32(buf, ret, width, height);
		}
		else if (from == TexFmt_ETC2_RGB4) {
			auto size = width * height * 4;
			ret = malloc(width * height * 4);
			needFree = true;

			ETC2RGB4ToRGBA32(buf, ret, width, height);
		}
		//else if (from == TexFmt_RGBA4444) {
		//	//TODO 使用更好的转换方式
		//	auto size = width * height * 4;
		//	ret = malloc(width * height * 4);
		//	needFree = true;

		//	RGBA42RGBA(buf, ret, width * height);
		//} 
		else {
			char szTemp[1024];
			sprintf_s(szTemp, "unsupport from format %d\n", from);
			OutputDebugStringA(szTemp);
		}
	}
	else if (to == TexFmt_ARGB32) {
		if (from == TexFmt_RGBA32) {
			auto size = width * height * 4;
			ret = malloc(width * height * 4);
			needFree = true;

			RGBA2ARGB(buf, ret, width * height);
		}
	}

	return ret;
}