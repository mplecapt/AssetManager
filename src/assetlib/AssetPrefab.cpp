#include "AssetPrefab.h"
#include <json.hpp>
#include <lz4.h>

using namespace SiegeEngine::Assets;
using json = nlohmann::json;

PrefabInfo PrefabInfo::Read(AssetFile* file) {
	PrefabInfo info;
	json prefabMeta = json::parse(file->json);

	for (auto pair : prefabMeta["node_matrices"].items()) {
		auto value = pair.value();
		auto k = pair.key();
		info.node_matrices[value[0]] = value[1];
	}

	for (auto& [key, value] : prefabMeta["node_names"].items()) {
		info.node_names[value[0]] = value[1];
	}

	for (auto& [key, value] : prefabMeta["node_parents"].items()) {
		info.node_parents[value[0]] = value[1];
	}

	std::unordered_map<uint64_t, json> meshnodes = prefabMeta["node_meshes"];

	for (auto pair : meshnodes) {
		PrefabInfo::NodeMesh node;

		node.mesh_path = pair.second["mesh_path"];
		node.material_path = pair.second["material_path"];

		info.node_meshes[pair.first] = node;
	}

	size_t nmatrices = file->binaryBlob.size() / (sizeof(float) * 16);
	info.matrices.resize(nmatrices);

	memcpy(info.matrices.data(), file->binaryBlob.data(), file->binaryBlob.size());
	return info;
}

AssetFile PrefabInfo::Pack(const PrefabInfo& info) {
	json prefabMeta;
	prefabMeta["node_matrices"] = info.node_matrices;
	prefabMeta["node_names"] = info.node_names;
	prefabMeta["node_parents"] = info.node_parents;
	
	std::unordered_map<uint64_t, json> meshIndex;
	for (auto pair : info.node_meshes) {
		json meshnode;
		meshnode["mesh_path"] = pair.second.mesh_path;
		meshnode["material_path"] = pair.second.material_path;
		meshIndex[pair.first] = meshnode;
	}

	prefabMeta["node_meshes"] = meshIndex;

	AssetFile file;
	file.type[0] = 'P';
	file.type[1] = 'R';
	file.type[2] = 'F';
	file.type[3] = 'B';
	file.version = 1;

	file.binaryBlob.resize(info.matrices.size() * sizeof(float) * 16);
	memcpy(file.binaryBlob.data(), info.matrices.data(), info.matrices.size() * sizeof(float) * 16);

	std::string stringified = prefabMeta.dump();
	file.json = stringified;

	return file;
}