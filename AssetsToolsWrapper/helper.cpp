#pragma once

#include "helper.h"
#include <memory>
#include "AssetsTools/AssetTypeClass.h"
#include "endian_rw.h"
#include <iomanip>
#include <sstream>
#include "lz4/lz4hc.h"
#include "lzma/LzmaEnc.h"
#include "lzma/Alloc.h"

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

extern std::string g_tempFile;

#define tmpPngFile ((g_tempFile + ".png").c_str())

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

void* convertTextureFormat(void* buf, int width, int height, bool& needFree, TextureFormat from, TextureFormat to);

bool getPngBuf(std::auto_ptr<char> &pngBuf, size_t& size, void * buf, int width, int height, TextureFormat format) {
	FILE* fp = NULL;
	png_structp png = NULL;
	png_infop info = NULL;
	bool needFree = false;
	buf = convertTextureFormat(buf, width, height, needFree, format, TexFmt_RGBA32);
	if (!buf) {
		return false;
	}
	auto img = static_cast<png_byte*>(buf);
	auto ret = false;
	do {
		fopen_s(&fp, tmpPngFile, "wb");
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

	if (ret) {
		fopen_s(&fp, tmpPngFile, "rb");
		if (!fp) return false;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		pngBuf.reset(new char[size]);
		fseek(fp, 0, SEEK_SET);
		fread(pngBuf.get(), 1, size, fp);
		fclose(fp);
	}

	if (needFree && buf) {
		free(buf);
	}

	return ret;
}

bool readPng(const char *fn, std::auto_ptr<char> &buf, int &width, int &height, TextureFormat format) {
	FILE* fp = NULL;
	png_structp png = NULL;
	png_infop info = NULL, endInfo = NULL;;
	png_byte* img = NULL;
	bool needFree = false;
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

		buf.reset((char*)convertTextureFormat(img, width, height, needFree, TexFmt_RGBA32, format));

		if (buf.get() == NULL) {
			break;
		}

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
			img = NULL;
		}
	}

	if (needFree && img) {
		delete[]img;
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

void inc(char*& dst, char*& src, size_t size) {
	dst += size;
	src += size;
}

void scp(char*& dst, char*& src, size_t size) {
	memcpy(dst, src, size);
	inc(dst, src, size);
}

struct BlockInfo {
	uint32_t decompressedSize, compressedSize;
	uint16_t flag;
};

struct NodeInfo {
	uint64_t offset, size;
	uint32_t status;
	std::string name;
};

bool compressBlocks(std::istream& stream, const BlockInfo& block, const NodeInfo& node, std::string &blockBuffer, BlockInfo& outputBlock);

bool tryPackFile(const char *srcFile, const char *dstFile) {
	// 仅处理只有一个AssetBundle的问题
	std::ifstream src;
	std::ofstream dst;
	auto ret = false;
	src.open(srcFile, std::ios::binary);
	dst.open(dstFile, std::ios::binary);
	do {
		if (!src.is_open()) break;
		std::string sig, unity, generator;
		uint32_t fileVersion;
		uint64_t fileSize;
		int32_t compressedSize, decompressedSize;
		uint32_t flag;

		src > sig;
		src > fileVersion;
		src > unity;
		src > generator;
		src > fileSize;
		src > compressedSize;
		src > decompressedSize;
		src > flag;

		auto compressType = flag & 0x3f;
		if (compressType != 0) {
			ret = copy(src, dst);
			break;
		}
		// 压缩文件
		auto needBack = false;
		auto backPos = src.tellg();
		if (flag & 0x80) {
			src.seekg(-compressedSize, std::ios::end);
			needBack = true;
		}
		char guid[16];
		src.read(guid, 16);
		int32_t blockNum;
		src > blockNum;
		std::vector<BlockInfo> blocks;
		for(auto i=0; i<blockNum; i++) {
			BlockInfo block;
			src > block.compressedSize;
			src > block.decompressedSize;
			src > block.flag;
			blocks.push_back(block);
		}

		int32_t nodeNum;
		src > nodeNum;
		std::vector<NodeInfo> nodes;
		for(auto i=0; i<nodeNum; i++) {
			NodeInfo node;
			src > node.offset;
			src > node.size;
			src > node.status;
			src > node.name;
			nodes.push_back(node);
		}
		if (nodeNum != 1 || blockNum != 1 || nodeNum != blockNum) {
			ret = copy(src, dst);
			break;
		}
		// 压缩并输出文件
		std::string blockBuffer;
		if (needBack) {
			src.seekg(backPos, std::ios::beg);
		}

		auto& node = nodes[0];
		auto& block = blocks[0];

		BlockInfo outputBlock;
		
		if (!compressBlocks(src, block, node, blockBuffer, outputBlock)) {
			break;
		}
		
		dst > sig;
		dst.put(0);
		dst > fileVersion;
		dst > unity;
		dst.put(0);
		dst > generator;
		dst.put(0);

		flag = 0x43; //接在头后，使用lz4压缩
		node.offset = 0;
		node.size = outputBlock.decompressedSize;

		std::ostringstream dirBuffer;
		nodeNum = 1;
		blockNum = 1;
		dirBuffer.write(guid, 16);
		dirBuffer > blockNum;
		dirBuffer > outputBlock.decompressedSize;
		dirBuffer > outputBlock.compressedSize;
		dirBuffer > outputBlock.flag;
		dirBuffer > nodeNum;
		dirBuffer > node.offset;
		dirBuffer > node.size;
		dirBuffer > node.status;
		dirBuffer > node.name;
		dirBuffer.put(0);

		decompressedSize = dirBuffer.str().length();
		std::string compressDir;
		compressDir.resize(decompressedSize);
		compressedSize = LZ4_compress_HC(dirBuffer.str().data(), (char*)compressDir.data(), decompressedSize, decompressedSize, LZ4HC_CLEVEL_DEFAULT);

		fileSize = uint64_t(dst.tellp()) + sizeof(fileSize) + sizeof(compressedSize) + sizeof(decompressedSize)
			+ sizeof(flag) + compressedSize + blockBuffer.length();
		dst > fileSize;
		dst > compressedSize;
		dst > decompressedSize;
		dst > flag;
		// 输出目录
		dst.write(compressDir.data(), compressedSize);
		// 输出块信息
		dst > blockBuffer;
		ret = true;
	} while (false);

	return ret;
}

bool compressBlocks(std::istream& stream, const BlockInfo& block, const NodeInfo& node, std::string &blockBuffer, BlockInfo& outputBlock) {
	outputBlock.decompressedSize = block.decompressedSize;
	outputBlock.flag = 0x41;
	std::string src, dst;
	src.resize(outputBlock.decompressedSize);
	dst.resize(outputBlock.decompressedSize);
	auto ret = false;
	blockBuffer.clear();

	stream.seekg(node.offset, std::ios::cur);
	auto remain = block.decompressedSize;
	CLzmaEncProps props;
	auto enc = LzmaEnc_Create(&g_Alloc);
	if (enc == 0) {
		return ret;
	}
	LzmaEncProps_Init(&props);
	props.level = 9;
	props.lc = 3;
	props.lp = 0;
	props.pb = 2;
	props.dictSize = 0x80000;
	auto res = LzmaEnc_SetProps(enc, &props);

	if (res != SZ_OK) {
		return ret;
	}

	do {
		Byte header[LZMA_PROPS_SIZE + 8];
		size_t headerSize = LZMA_PROPS_SIZE;
		res = LzmaEnc_WriteProperties(enc, header, &headerSize);
		blockBuffer.append((char*)header, headerSize);
		SizeT dstSize = block.decompressedSize;
		stream.read((char*)src.data(), block.decompressedSize);
		res = LzmaEnc_MemEncode(enc, (Byte*)dst.data(), &dstSize, (Byte*)src.data(), block.decompressedSize, 0, NULL, &g_Alloc, &g_Alloc);
		blockBuffer.append(dst.data(), dstSize);
		ret = res == SZ_OK;
	} while (false);

	if (enc) {
		LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
	}

	outputBlock.compressedSize = blockBuffer.length();
	return ret;
}