#include "AssetTexture.h"
#include <fstream>
#include <json.hpp>
#define LZ4_STATIC_LINKING_ONLY
#include <lz4.h>

using namespace SiegeEngine::Assets;
using json = nlohmann::json;

TextureFormat ParseFormat(const char* f) {
	if (strcmp(f, "RGBA8") == 0)
		return TextureFormat::RGBA8;
	else
		return TextureFormat::UNKNOWN;
}

TextureInfo TextureInfo::Read(AssetFile* file) {
	TextureInfo info;

	json texture_metadata = json::parse(file->json);

	std::string formatString = texture_metadata["format"];
	info.textureFormat = ParseFormat(formatString.c_str());

	std::string compressionString = texture_metadata["compression"];
	info.compressionMode = ParseCompression(compressionString.c_str());

	info.pixelsize[0] = texture_metadata["width"];
	info.pixelsize[1] = texture_metadata["height"];
	info.textureSize = texture_metadata["buffer_size"];
	info.originalFile = texture_metadata["original_file"];

	return info;
}

void TextureInfo::Unpack(TextureInfo* info, const char* srcbuffer, size_t srcsize, char* dst) {
	if (info->compressionMode == CompressionMode::LZ4) {
		LZ4_decompress_safe(srcbuffer, dst, (int)srcsize, (int)info->textureSize);
	} else {
		memcpy(dst, srcbuffer, srcsize);
	}
}

AssetFile TextureInfo::Pack(TextureInfo* info, void* pixelData) {
	json texture_metadata;
	texture_metadata["format"] = "RGBA8";
	texture_metadata["width"] = info->pixelsize[0];
	texture_metadata["height"] = info->pixelsize[1];
	texture_metadata["buffer_size"] = info->textureSize;
	texture_metadata["original_file"] = info->originalFile;

	AssetFile file;
	file.type[0] = 'T';
	file.type[1] = 'E';
	file.type[2] = 'X';
	file.type[3] = 'I';
	file.version = 1;

	int compressStaging = LZ4_compressBound((int)info->textureSize);

	file.binaryBlob.resize(compressStaging);

	int compressedSize = LZ4_compress_default((const char*)pixelData, file.binaryBlob.data(), (int)info->textureSize, compressStaging);

	file.binaryBlob.resize(compressedSize);

	texture_metadata["compression"] = "LZ4";

	std::string stringified = texture_metadata.dump();
	file.json = stringified;

	return file;
}