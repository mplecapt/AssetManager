#include "AssetMaterial.h"
#include <json.hpp>
#include <lz4.h>

using namespace SiegeEngine::Assets;
using json = nlohmann::json;

MaterialInfo MaterialInfo::Read(AssetFile* file) {
	MaterialInfo info;

	json materialMeta = json::parse(file->json);
	info.baseEffect = materialMeta["baseEffect"];

	for (auto& [key, value] : materialMeta["textures"].items())
		info.textures[key] = value;

	for (auto& [key, value] : materialMeta["customProperties"].items())
		info.customProperties[key] = value;

	info.transparency = TransparencyMode::OPAQUE;

	auto it = materialMeta.find("transparency");
	if (it != materialMeta.end()) {
		std::string val = (*it);
		if (val.compare("transparent") == 0)
			info.transparency = TransparencyMode::TRANSPARENT;
		if (val.compare("masked") == 0)
			info.transparency = TransparencyMode::MASKED;
	}

	return info;
}

AssetFile MaterialInfo::Pack(MaterialInfo* info) {
	json materialMeta;
	materialMeta["baseEffect"] = info->baseEffect;
	materialMeta["textures"] = info->textures;
	materialMeta["customProperties"] = info->customProperties;

	switch (info->transparency) {
		case TransparencyMode::TRANSPARENT:
			materialMeta["transparency"] = "transparent";
			break;
		case TransparencyMode::MASKED:
			materialMeta["transparency"] = "masked";
			break;
	}

	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'A';
	file.type[2] = 'T';
	file.type[3] = 'X';
	file.version = 1;

	std::string stringified = materialMeta.dump();
	file.json = stringified;

	return file;
}