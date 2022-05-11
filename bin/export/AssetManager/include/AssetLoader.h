#pragma once
#include <string>
#include <vector>

namespace SiegeEngine::Assets {

	struct AssetFile {
		char type[4];
		int version;
		std::string json;
		std::vector<char> binaryBlob;

		static bool Save(const char* path, const AssetFile& file);
		static bool Load(const char* path, AssetFile& outputFile);
	};
	
	enum class CompressionMode : uint32_t {
		NONE = 0,
		LZ4
	};
	
	extern CompressionMode ParseCompression(const char* f);

}