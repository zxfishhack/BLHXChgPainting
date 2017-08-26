#pragma once

#include "helper.h"
#include <memory>
#include "AssetsTools/AssetTypeClass.h"
#include <iomanip>
#include <sstream>

QWORD _cdecl writer(QWORD pos, QWORD count, const void *pBuf, LPARAM par) {
	if (memcmp(pBuf, "CAB-", 4) == 0) {
		par = par;
	}
	auto fp = reinterpret_cast<FILE*>(par);
	fseek(fp, long(pos), SEEK_SET);
	return fwrite(pBuf, 1, count, fp);
}

png_voidp PNGCBAPI png_malloc(png_structp png, png_alloc_size_t size) {
	return malloc(size);
}

void PNGCBAPI png_free(png_structp png, png_voidp ptr) {
	free(ptr);
}

bool writePng(const char * fn, void * buf, int width, int height) {
	std::string debugInfo = "writing ";
	debugInfo += fn;
	OutputDebugStringA(debugInfo.c_str());
	FILE* fp = NULL;
	png_structp png = NULL;
	png_infop info = NULL;
	auto img = static_cast<png_byte*>(buf);
	auto ret = false;
	do {
		fopen_s(&fp, fn, "wb");
		if (!fp)
			break;
		png = png_create_write_struct(png_get_libpng_ver(NULL), NULL, NULL, NULL);
		if (!png)
			break;

		png_set_mem_fn(png, NULL, png_malloc, png_free);

		png_init_io(png, fp);

		info = png_create_info_struct(png);
		if (!info)
			break;

		if (setjmp(png_jmpbuf(png)))
			break;

		png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png, info);

		auto_ptr<png_bytep> prows(new png_bytep[height]);

		for (auto i = 0; i<height; i++) {
			prows.get()[i] = img + (height - i - 1) * width * 4;
		}

		png_write_image(png, prows.get());

		png_write_end(png, info);

		ret = true;
	} while (false);

	if (png) {
		png_destroy_write_struct(&png, &info);
	}

	if (fp) {
		fclose(fp);
	}

	return ret;
}

bool readPng(const char *fn, std::auto_ptr<char> &buf, int &width, int &height) {
	std::string debugInfo = "reading ";
	debugInfo += fn;
	OutputDebugStringA(debugInfo.c_str());
	FILE* fp = NULL;
	png_structp png = NULL;
	png_infop info = NULL, endInfo = NULL;;
	png_byte* img = NULL;
	auto ret = false;
	do {
		fopen_s(&fp, fn, "rb");
		if (!fp)
			break;
		png_byte sig[8];

		fread(sig, 1, 8, fp);
		if (png_sig_cmp(sig, 0, 8))
			break;
		png = png_create_read_struct(png_get_libpng_ver(NULL), NULL, NULL, NULL);
		if (!png)
			break;

		png_set_mem_fn(png, NULL, png_malloc, png_free);

		png_init_io(png, fp);

		png_set_sig_bytes(png, 8);

		info = png_create_info_struct(png);
		if (!info)
			break;

		endInfo = png_create_info_struct(png);
		if (!endInfo)
			break;

		if (setjmp(png_jmpbuf(png)))
			break;

		png_read_info(png, info);

		png_uint_32 w, h;
		int depth, colorType;
		png_get_IHDR(png, info, &w, &h, &depth, &colorType, NULL, NULL, NULL);
		width = w;
		height = h;
		if (depth != 8 || colorType != PNG_COLOR_TYPE_RGBA)
			break;

		img = new png_byte[width * height * 4];

		auto_ptr<png_bytep> prows(new png_bytep[height]);

		for (auto i = 0; i<height; i++) {
			prows.get()[i] = img + (height - i - 1) * width * 4;
		}

		png_read_image(png, prows.get());
		buf.reset((char*)img);

		png_read_end(png, endInfo);

		ret = true;
	} while (false);

	if (png) {
		png_destroy_read_struct(&png, &info, &endInfo);
	}

	if (fp) {
		fclose(fp);
	}

	if (!ret) {
		if (img) {
			delete[]img;
		}
	}

	return ret;
}

void memcpy2D(void *dst_, size_t dstPitch, void *src_, size_t srcPitch, size_t size, size_t count) {
	auto src = static_cast<char*>(src_), dst = static_cast<char*>(dst_);
	for(auto i=0; i<count; i++) {
		memcpy(dst + i * dstPitch, src + i * srcPitch, size);
	}
}

void dfs(AssetTypeValueField* tv, int offset, int step) {
	static std::ostringstream ss;
	ss.str("");
	ss << std::setw(offset) << setfill(' ') << '-';
	ss << tv->GetType() << " " << tv->GetName();
	auto v = tv->GetValue();
	if (v && v->GetType() != ValueType_None) {
		ss << " : ";
		switch (v->GetType()) {
		case ValueType_Bool:
			ss << v->AsBool();
			break;
		case ValueType_Float:
			ss << v->AsFloat();
			break;
		case ValueType_Double:
			ss << v->AsDouble();
			break;
		case ValueType_String:
			ss << v->AsString();
			break;
		case ValueType_Array:
			ss << "array value";
			break;
		case ValueType_ByteArray:
			ss << "byte array value";
			break;
		default:
			ss << v->AsInt();
			break;
		}
	}
	ss << std::endl;
	OutputDebugStringA(ss.str().c_str());
	for (DWORD i = 0; i<tv->GetChildrenCount(); i++) {
		dfs(tv->GetChildrenList()[i], offset + step, step);
	}
}

bool repack(const char * src, const char* dst) {
	FILE* fp;
	auto ret = false;
	do {
		fopen_s(&fp, dst, "wb");
		if (!fp) break;
		file f;
		if (!f.open(src)) break;
		AssetsBundleFile bf;
		ret = bf.Pack(file::reader, f, writer, LPARAM(fp));
	} while (false);

	if (fp) {
		fclose(fp);
	}

	return ret;
}

void inc(char*& dst, char*& src, size_t size) {
	dst += size;
	src += size;
}

void scp(char*& dst, char*& src, size_t size) {
	memcpy(dst, src, size);
	inc(dst, src, size);
}

unsigned int _ntohl(unsigned int b) {
	auto a = b & 0xff;
	a <<= 8;
	b >>= 8;
	a |= b & 0xff;
	a <<= 8;
	b >>= 8;
	a |= b & 0xff;
	a <<= 8;
	b >>= 8;
	a |= b & 0xff;
	return a;
}

void postFix(const char *srcFile, const char *dstFile) {
	FILE* fp = NULL;
	do {
		fopen_s(&fp, srcFile, "rb");
		if (!fp) break;
		fseek(fp, 0, SEEK_END);
		auto length = ftell(fp);
		auto remain = length;
		std::auto_ptr<char> buf(new char[length]);
		auto src = buf.get();
		// 原因未知，使用AssetsTool.dll重新打包的文件，会多三个字节
		std::auto_ptr<char> outBuf(new char[length - 3]);
		auto dst = outBuf.get();
		fseek(fp, 0, SEEK_SET);
		fread(buf.get(), 1, length, fp);
		fclose(fp);
		scp(dst, src, 0x1e);
		remain -= 0x1e;
		// 调整大小
		auto size = _ntohl(*(unsigned int*)(buf.get() + 0x1e));
		size -= 3;
		memcpy(dst, &size, 4);
		remain -= 4;
		inc(src, dst, 4);
		scp(dst, src, 53);
		remain -= 4;
		*dst = 0;
		//skip 03
		inc(dst, src, 1);
		scp(dst, src, 49);
		//跳过多余的三个字节
		src += 3;
		remain -= 3;
		scp(dst, src, remain);
		fopen_s(&fp, dstFile, "wb");
		if (!fp) break;
		fwrite(outBuf.get(), 1, length - 3, fp);
		fclose(fp);
	} while (false);
}
