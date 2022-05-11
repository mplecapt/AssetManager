#pragma once
#include "AssetLoader.h"
#include <string>

namespace SiegeEngine::Assets {

	enum class TextureFormat : uint32_t {
		UNKNOWN = 0,
		RGBA8
	};

	struct TextureInfo {
		uint64_t textureSize;
		TextureFormat textureFormat;
		CompressionMode compressionMode;
		uint32_t pixelsize[3];
		std::string originalFile;

		static TextureInfo Read(AssetFile* file);
		static void Unpack(TextureInfo* info, const char* srcbuffer, size_t srcsize, char* dst);
		static AssetFile Pack(TextureInfo* info, void* pixelData);
	};

}