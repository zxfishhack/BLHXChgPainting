#include <Windows.h>
#include <cstdio>

#pragma comment(lib, "AssetsTools.lib")
#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpng16d.lib")
#else
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")
#endif

#include <iomanip>
#include "color_process.h"
#include "helper.h"
#include <map>
#include "AssetsTools/TextureFileFormat.h"

std::string g_curHash, g_lastHash;
std::string g_tempFile;
std::map<std::string, std::string> textures;

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	switch(fdwReason) {
	case DLL_PROCESS_ATTACH: 
	{
		char szTempPath[1024];
		GetTempPath(1024, szTempPath);
		g_tempFile = szTempPath;
		g_tempFile += "BLHXChgPainting";
	}
	break;
	default: break;
	}
	return TRUE;
}

void _cdecl verifyLog(char *message) {
#ifdef _DEBUG
	std::string msg(message);
	msg += "\n";
	OutputDebugStringA(msg.c_str());
#endif
}

bool check_open(const char* fn_, file& f, bool& same) {
	std::string fn(fn_);
	AssetsBundleFile bf;
	auto ret = false;
	do {
		if (!f.open(fn_) || !bf.Read(file::reader, f, verifyLog, true))
			break;
		g_curHash = f.md5;
		if (bf.bundleHeader6.compressedSize != bf.bundleHeader6.decompressedSize) {
			FILE* fo = NULL;
			fn = g_tempFile;
			fopen_s(&fo, fn.c_str(), "rb");
			if (g_curHash == g_lastHash && fo) {
				fclose(fo);
				ret = f.open(fn.c_str());
				break;
			}
			if (fo) {
				fclose(fo);
			}
			fopen_s(&fo, fn.c_str(), "wb");
			if (!fo)
				break;
			if (!bf.Unpack(file::reader, f, writer, LPARAM(fo)))
				break;
			if (fo) {
				fclose(fo);
			}
			repack(fn.c_str(), "d:\\test_enc_tex");
			ret = f.open(fn.c_str());
		} else {
			ret = true;
		}
	} while (false);
	same = g_curHash == g_lastHash;
	if (ret) {
		g_lastHash = g_curHash;
	} else {
		g_curHash = "1";
		g_lastHash = "2";
	}
	return ret;
}

bool ExtraPngFile(const char* fn, const char* textureName) {
	AssetsBundleFile bf;
	file infile;
	bool same;
	if (!check_open(fn, infile, same) || !bf.Read(file::reader, infile, verifyLog, false)) {
		return false;
	}
	if (same && textures.find(textureName) != textures.end()) {
		return true;
	}
	if (!same) {
		textures.clear();
	}
	if (bf.bundleHeader6.fileVersion != 6) {
		return false;
	}
	LPARAM p(infile);
	if (!bf.IsAssetsFile(file::reader, infile, bf.bundleInf6->dirInf)) {
		return false;
	}
	auto reader = bf.MakeAssetsFileReader(file::reader, &p, bf.bundleInf6->dirInf);
	if (!reader) {
		return false;
	}
	AssetsFileReaderAutoFree _(reader, p);
	AssetsFile assetsFile(reader, p);
	if (!assetsFile.VerifyAssetsFile(verifyLog) || !assetsFile.typeTree.hasTypeTree) {
		return false;
	}
	AssetsFileTable assetsFileTable(&assetsFile);
	for (DWORD i = 0; i<assetsFileTable.assetFileInfoCount; i++) {
		auto assetFileInfoEx = assetsFileTable.pAssetFileInfo[i];
		bool isEncoded = strstr(assetFileInfoEx.name, "_enc") != NULL;
		if (assetFileInfoEx.curFileType == 0x1c && strcmp(assetFileInfoEx.name, textureName) == 0) {
			AssetTypeTemplateField root;
			auto pr = &root;
			// texture2D
			for(DWORD j=0; j<assetsFile.typeTree.fieldCount; j++) {
				if (assetsFile.typeTree.pTypes_Unity5[j].classId == assetFileInfoEx.inheritedUnityClass) {
					root.From0D(assetsFile.typeTree.pTypes_Unity5 + j, 0);
					break;
				}
			}
			std::auto_ptr<char> buf(new char[assetFileInfoEx.curFileSize]);
			if (reader(assetFileInfoEx.absolutePos, assetFileInfoEx.curFileSize, buf.get(), p) == assetFileInfoEx.curFileSize) {
				auto assetValueReader = Create_AssetsReaderFromMemory(buf.get(), assetFileInfoEx.curFileSize, false);
				AssetTypeInstance inst(1, &pr, AssetsReaderFromMemory, assetValueReader, false);
				auto& rtv = *(inst.GetBaseField());
				auto imgData = rtv["image data"]->GetValue()->AsByteArray();
				std::auto_ptr<char> imgBuf(new char[imgData->size]);
				auto width = rtv["m_Width"]->GetValue()->AsInt(), height = rtv["m_Height"]->GetValue()->AsInt();
				auto textureFormat = (TextureFormat)rtv["m_TextureFormat"]->GetValue()->AsInt();
				memcpy(imgBuf.get(), imgData->data, imgData->size);
				if (isEncoded) {
					blhx::decode(imgBuf.get(), width, height);
				}
				std::auto_ptr<char> pngBuf;
				size_t size;
				if (getPngBuf(pngBuf, size, imgBuf.get(), width, height, textureFormat)) {
					textures[textureName].assign(pngBuf.get(), size);
					return true;
				}
			}
		}
	}
	return false;
}

std::string result;

const char* GetTextureList(const char* fn) {
	OutputDebugStringA(fn);
	AssetsBundleFile bf;
	file infile;
	bool same;
	if (!check_open(fn, infile, same) || !bf.Read(file::reader, infile, verifyLog, false)) {
		return NULL;
	}
	if (same) {
		return result.c_str();
	}
	if (!same) {
		textures.clear();
	}
	if (bf.bundleHeader6.fileVersion != 6) {
		return NULL;
	}
	LPARAM p(infile);
	if (!bf.IsAssetsFile(file::reader, infile, bf.bundleInf6->dirInf)) {
		return false;
	}
	auto reader = bf.MakeAssetsFileReader(file::reader, &p, bf.bundleInf6->dirInf);
	if (!reader) {
		return NULL;
	}
	AssetsFileReaderAutoFree _(reader, p);
	AssetsFile assetsFile(reader, p);
	if (!assetsFile.VerifyAssetsFile(verifyLog) || !assetsFile.typeTree.hasTypeTree) {
		return NULL;
	}
	AssetsFileTable assetsFileTable(&assetsFile);
	result.clear();
	for(DWORD i=0; i<assetsFileTable.assetFileInfoCount; i++) {
		auto assetFileInfoEx = assetsFileTable.pAssetFileInfo[i];
		if (assetFileInfoEx.curFileType == 0x1c) {
			if (!result.empty()) {
				result += ",";
			}
			result += assetFileInfoEx.name;
		}
	}
	return result.c_str();
}

bool GetImageInfo(const char* fn, int& bufSize, const char* textureName) {
	if (!ExtraPngFile(fn, textureName)) {
		return false;
	}
	if (textures.find(textureName) == textures.end()) {
		return false;
	}
	bufSize = int(textures[textureName].length());
	return true;
}

bool LoadImageFromBundle(const char* fn, void* buf, const char* textureName) {
	file f;
	bool same;
	if (!check_open(fn, f, same)) {
		return false;
	}
	auto it = textures.find(textureName);
	if (same && it != textures.end()) {
		memcpy(buf, it->second.data(), it->second.length());
		return true;
	}
	if (!ExtraPngFile(fn, textureName)) {
		return false;
	}
	it = textures.find(textureName);
	if (it != textures.end()) {
		memcpy(buf, it->second.data(), it->second.length());
		return true;
	}
	return false;
}

#define tempAssetFile ((g_tempFile + ".assets").c_str())

bool ReplaceImageFile(const char* fn, const char* png, const char* textureName) {
	std::auto_ptr<char> pngBuf;
	int pngWidth, pngHeight;
	if (!readPng(png, pngBuf, pngWidth, pngHeight, TexFmt_RGBA32))
		return false;
	AssetsBundleFile bf;
	file infile;
	bool same;
	if (!check_open(fn, infile, same) || !bf.Read(file::reader, infile, verifyLog, false)) {
		return false;
	}
	if (bf.bundleHeader6.fileVersion != 6) {
		return false;
	}
	LPARAM p(infile);
	if (!bf.IsAssetsFile(file::reader, infile, bf.bundleInf6->dirInf)) {
		return false;
	}
	auto reader = bf.MakeAssetsFileReader(file::reader, &p, bf.bundleInf6->dirInf);
	if (!reader)
		return false;
	AssetsFileReaderAutoFree _(reader, p);
	AssetsFile assetsFile(reader, p);
	if (!assetsFile.VerifyAssetsFile(verifyLog) || !assetsFile.typeTree.hasTypeTree) {
		return false;
	}
	AssetsFileTable assetsFileTable(&assetsFile);
	for (DWORD i = 0; i<assetsFileTable.assetFileInfoCount; i++) {
		auto assetFileInfoEx = assetsFileTable.pAssetFileInfo[i];
		if (assetFileInfoEx.curFileType == 0x1c && strcmp(assetFileInfoEx.name, textureName) == 0) {
			AssetTypeTemplateField root;
			auto pr = &root;
			// texture2D
			for (DWORD j = 0; j<assetsFile.typeTree.fieldCount; j++) {
				if (assetsFile.typeTree.pTypes_Unity5[j].classId == assetFileInfoEx.inheritedUnityClass) {
					root.From0D(assetsFile.typeTree.pTypes_Unity5 + j, 0);
					break;
				}
			}
			std::auto_ptr<char> buf(new char[assetFileInfoEx.curFileSize]);
			if (reader(assetFileInfoEx.absolutePos, assetFileInfoEx.curFileSize, buf.get(), p) == assetFileInfoEx.curFileSize) {
				auto assetValueReader = Create_AssetsReaderFromMemory(buf.get(), assetFileInfoEx.curFileSize, false);
				AssetTypeInstance inst(1, &pr, AssetsReaderFromMemory, assetValueReader, false);
				auto& rtv = *(inst.GetBaseField());
				auto imgData = rtv["image data"]->GetValue()->AsByteArray();
				auto width = rtv["m_Width"]->GetValue()->AsInt(), height = rtv["m_Height"]->GetValue()->AsInt();
				int format = TexFmt_RGBA32;
				rtv["m_TextureFormat"]->GetValue()->Set(&format);
				std::auto_ptr<char> imgBuf(new char[width * height * 4]);
				if (!same_order(pngWidth, width, pngHeight,height)) {
					return false;
				}
				auto x = abs(pngWidth - width) / 2, y = abs(pngHeight - height) / 2;
				auto srcPitch = pngWidth * 4;
				auto dstPitch = width * 4;
				auto src = pngBuf.get();
				auto dst = imgBuf.get();
				auto count = height;
				auto elemSize = width;
				if (pngWidth < width || pngHeight < height) {
					// 替换图比目标图要小
					memset(imgBuf.get(), 0, imgData->size);
					dst += y * dstPitch + x * 4;
					count = pngHeight;
					elemSize = pngWidth;
				} else {
					src += y * srcPitch + x * 4;
				}
				memcpy2D(dst, dstPitch, src, srcPitch, elemSize * 4, count);
				bool isEncoded = strstr(assetFileInfoEx.name, "_enc") != NULL;
				if (isEncoded) {
					blhx::encode(imgBuf.get(), width, height);
				}
				AssetTypeByteArray byteArrayData;
				byteArrayData.size = imgData->size;
				byteArrayData.data = reinterpret_cast<BYTE*>(imgBuf.get());
				rtv["image data"]->GetValue()->Set(&byteArrayData);
				std::auto_ptr<char> replacedBuf(new char[rtv.GetByteSize()]);
				auto replacedParam = Create_AssetsWriterToMemory(replacedBuf.get(), rtv.GetByteSize());
				if (replacedParam == NULL) return false;
				FILE* fp;
				// 修改assets file
				auto newByteSize = rtv.Write(AssetsWriterToMemory, replacedParam, 0);
				fopen_s(&fp, tempAssetFile, "wb");
				if (!fp) return false;
				auto replacor = MakeAssetModifierFromMemory(0, assetFileInfoEx.index, assetFileInfoEx.curFileType, assetFileInfoEx.scriptIndex, replacedBuf.get(), newByteSize, NULL);
				AssetsReplacer* replacors[1];
				replacors[0] = replacor;
				if (!assetsFile.Write(writer, LPARAM(fp), 0, replacors, 1, -1)) {
					FreeAssetsReplacer(replacor);
					fclose(fp);
					return false;
				}
				FreeAssetsReplacer(replacor);
				fclose(fp);
				// 修改bundle file
				fopen_s(&fp, tempAssetFile, "rb");
				if (!fp) return false;
				fseek(fp, 0, SEEK_END);
				auto length = ftell(fp);
				std::auto_ptr<char> assetBuf(new char[length]);
				fseek(fp, 0, SEEK_SET);
				fread(assetBuf.get(), 1, length, fp);
				fclose(fp);
				fopen_s(&fp, tempAssetFile, "wb");
				auto breplacor = MakeBundleEntryModifierFromMem(bf.bundleInf6->dirInf[0].name, bf.bundleInf6->dirInf[0].name, true, assetBuf.get(), length);
				BundleReplacer* breplacers[1];
				breplacers[0] = breplacor;
				auto ret = bf.Write(file::reader, infile, writer, LPARAM(fp), breplacers, 1, verifyLog);
				FreeBundleReplacer(breplacor);
				fclose(fp);
				if (ret) {
					postFix(tempAssetFile, fn);
				}
#if 0
				// 重新打包文件
				fopen_s(&fp, fn, "wb");
				file f;
				f.open((g_tempFile + ".assets").c_str());
				AssetsBundleFile bf2;
				auto ret = bf2.Pack(file::reader, f, writer, LPARAM(fp));
				fclose(fp);
#endif
				return ret;
			}
		}
	}
	return false;
}
