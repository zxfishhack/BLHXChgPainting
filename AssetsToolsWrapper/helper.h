#pragma once
#include <string>
#include "md5.h"

#include "AssetsTools/AssetsFileReader.h"
#include "AssetsTools/AssetsBundleFileFormat.h"
#include "AssetsTools/AssetsFileReader.h"
#include "AssetsTools/AssetsBundleFileFormat.h"
#include "AssetsTools/AssetsFileFormat.h"
#include "AssetsTools/AssetsFileTable.h"
#include "AssetsTools/AssetTypeClass.h"
#include "AssetsTools/TextureFileFormat.h"

class AssetTypeValueField;

extern "C" {
#include "png.h"
}

struct file {
	LPARAM p;
	size_t length;
	std::string md5;
	file() : p(0), length(0) {}
	~file() {
		if (p) {
			Free_AssetsReaderFromMemory(p, true);
		}
	}
	bool open(const char* fn) {
		if (p) {
			Free_AssetsReaderFromMemory(p, true);
			length = 0;
			p = 0;
		}
		FILE* fp;
		fopen_s(&fp, fn, "rb");
		if (!fp) return false;
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		auto buf = new char[length];
		fseek(fp, 0, SEEK_SET);
		fread(buf, 1, length, fp);
		MD5 bmd5;
		bmd5.setMessage(buf, length);
		md5 = bmd5.getResult();
		p = Create_AssetsReaderFromMemory(buf, length, true);
		delete[]buf;
		fclose(fp);
		return p != 0;
	}
	operator LPARAM() const {
		return LPARAM(p);
	}

	QWORD static _cdecl reader(QWORD pos, QWORD count, void *pBuf, LPARAM par) {
		return AssetsReaderFromMemory(pos, count, pBuf, par);
	}
};

class AssetsFileReaderAutoFree {
public:
	AssetsFileReaderAutoFree(AssetsFileReader reader, LPARAM p) : _reader(reader), _p(p) {}
	~AssetsFileReaderAutoFree() {
		FreeAssetsBundle_FileReader(&_p, &_reader);
	}
private:
	AssetsFileReader _reader;
	LPARAM _p;
};

template<typename Tl, typename Tr>
int same_order_prefix(Tl&& lhs, Tr&& rhs) {
	return lhs < rhs ? -1 : (rhs < lhs ? 1 : 0);
}

template<typename Tl, typename Tr>
bool same_order_with_prefix(int prefix, Tl&& lhs, Tr&& rhs) {
	auto curprefix = same_order_prefix(std::forward<Tl&&>(lhs), std::forward<Tr&&>(rhs));;
	return prefix == 0 ? true : curprefix == 0 ? true : curprefix == prefix;
}

template <typename Tl, typename Tr, typename ...T>
bool same_order_with_prefix(int prefix, Tl&& lhs, Tr&& rhs, T&&... tail) {
	auto curprefix = same_order_prefix(std::forward<Tl&&>(lhs), std::forward<Tr&&>(rhs));
	return prefix == 0 ? same_order_with_prefix(curprefix, std::forward<T&&>(tail)...) :
		(curprefix == 0 ? same_order_with_prefix(prefix, std::forward<T&&>(tail)...) : (
			prefix == curprefix ? same_order_with_prefix(prefix, std::forward<T&&>(tail)...) : false
			));
}

template <typename ...T>
bool same_order(T&&... tail) {
	return same_order_with_prefix(0, std::forward<T&&>(tail)...);
}

QWORD _cdecl writer(QWORD pos, QWORD count, const void *pBuf, LPARAM par);

bool getPngBuf(std::auto_ptr<char> &pngBuf, size_t& size, void * buf, int width, int height, TextureFormat format);
bool readPng(const char * fn, std::auto_ptr<char>& buf, int& width, int& height, TextureFormat format);

void memcpy2D(void *dst_, size_t dstPitch, void *src_, size_t srcPitch, size_t size, size_t count);

void dfs(AssetTypeValueField* tv, int offset, int step = 3);

bool tryPackFile(const char *src, const char *dst);