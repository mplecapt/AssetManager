#pragma once
#include "AssetLoader.h"
#include <unordered_map>

namespace SiegeEngine::Assets {

enum class TransparencyMode : uint8_t {
	OPAQUE,
	TRANSPARENT,
	MASKED
};

struct MaterialInfo {
	std::string baseEffect;
	std::unordered_map<std::string, std::string> textures;
	std::unordered_map<std::string, std::string> customProperties;
	TransparencyMode transparency;

	static MaterialInfo Read(AssetFile* file);
	static AssetFile Pack(MaterialInfo* info);
};

}