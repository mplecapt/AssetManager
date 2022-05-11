#include "AssetMesh.h"
#include <json.hpp>
#include <lz4.h>

using namespace SiegeEngine::Assets;
using json = nlohmann::json;

VertexFormat ParseFormat(const char* f) {
	if (strcmp(f, "PNCV_F32") == 0)
		return VertexFormat::PNCV_F32;
	else if (strcmp(f, "P32N8C8V16") == 0)
		return VertexFormat::P32N8C8V16;
	else
		return VertexFormat::UNKNOWN;
}

MeshInfo MeshInfo::Read(AssetFile* file) {
	MeshInfo info;

	json metadata = json::parse(file->json);

	info.vertexBufferSize = metadata["vertex_buffer_size"];
	info.indexBufferSize = metadata["index_buffer_size"];
	info.indexSize = (uint8_t) metadata["index_size"];
	info.originalFile = metadata["original_file"];

	std::string compressionString = metadata["compression"];
	info.compressionMode = ParseCompression(compressionString.c_str());

	std::vector<float> boundsData;
	boundsData.reserve(7);
	boundsData = metadata["bounds"].get<std::vector<float>>();

	info.bounds.origin[0] = boundsData[0];
	info.bounds.origin[1] = boundsData[1];
	info.bounds.origin[2] = boundsData[2];

	info.bounds.radius = boundsData[3];

	info.bounds.extents[0] = boundsData[4];
	info.bounds.extents[1] = boundsData[5];
	info.bounds.extents[2] = boundsData[6];

	std::string vertexFormat = metadata["vertex_format"];
	info.vertexFormat = ParseFormat(vertexFormat.c_str());
	
	return info;
}

void MeshInfo::Unpack(MeshInfo* info, const char* srcbuffer, size_t srcsize, char* vertexBuffer, char* indexBuffer) {
	std::vector<char> decompressedBuffer;
	decompressedBuffer.resize(info->vertexBufferSize + info->indexBufferSize);

	LZ4_decompress_safe(srcbuffer, decompressedBuffer.data(), static_cast<int>(srcsize), static_cast<int>(decompressedBuffer.size()));

	memcpy(vertexBuffer, decompressedBuffer.data(), info->vertexBufferSize);
	memcpy(indexBuffer, decompressedBuffer.data() + info->vertexBufferSize, info->indexBufferSize);
}

AssetFile MeshInfo::Pack(MeshInfo* info, char* vertexData, char* indexData) {
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'E';
	file.type[2] = 'S';
	file.type[3] = 'H';
	file.version = 1;

	json meta;
	if (info->vertexFormat == VertexFormat::P32N8C8V16)
		meta["vertex_format"] = "P32N8C8V16";
	else if (info->vertexFormat == VertexFormat::PNCV_F32)
		meta["vertex_format"] = "PNCV_F32";
	
	meta["vertex_buffer_size"] = info->vertexBufferSize;
	meta["index_buffer_size"] = info->indexBufferSize;
	meta["index_size"] = info->indexSize;
	meta["original_file"] = info->originalFile;

	std::vector<float> boundsData;
	boundsData.resize(7);

	boundsData[0] = info->bounds.origin[0];
	boundsData[1] = info->bounds.origin[1];
	boundsData[2] = info->bounds.origin[2];

	boundsData[3] = info->bounds.radius;

	boundsData[4] = info->bounds.extents[0];
	boundsData[5] = info->bounds.extents[1];
	boundsData[6] = info->bounds.extents[2];

	meta["bounds"] = boundsData;

	size_t fullsize = info->vertexBufferSize + info->indexBufferSize;

	std::vector<char> mergedBuffer;
	mergedBuffer.resize(fullsize);

	memcpy(mergedBuffer.data(), vertexData, info->vertexBufferSize);
	memcpy(mergedBuffer.data() + info->vertexBufferSize, indexData, info->indexBufferSize);

	size_t compressStaging = LZ4_compressBound(static_cast<int>(fullsize));
	file.binaryBlob.resize(compressStaging);

	int compressedSize = LZ4_compress_default(mergedBuffer.data(), file.binaryBlob.data(), static_cast<int>(mergedBuffer.size()), static_cast<int>(compressStaging));
	file.binaryBlob.resize(compressedSize);

	meta["compression"] = "LZ4";

	file.json = meta.dump();

	return file;
}

MeshBounds SiegeEngine::Assets::CalculateBounds(Vertex_f32_PNCV* vertices, size_t count) {
	MeshBounds bounds;

	float min[3] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	float max[3] = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

	for (size_t i = 0; i < count; i++) {
		min[0] = std::min(min[0], vertices[i].position[0]);
		min[1] = std::min(min[1], vertices[i].position[1]);
		min[2] = std::min(min[2], vertices[i].position[2]);

		max[0] = std::max(max[0], vertices[i].position[0]);
		max[1] = std::max(max[1], vertices[i].position[1]);
		max[2] = std::max(max[2], vertices[i].position[2]);
	}

	bounds.extents[0] = (max[0] - min[0]) / 2.0f;
	bounds.extents[1] = (max[1] - min[1]) / 2.0f;
	bounds.extents[2] = (max[2] - min[2]) / 2.0f;

	bounds.origin[0] = bounds.extents[0] + min[0];
	bounds.origin[1] = bounds.extents[1] + min[1];
	bounds.origin[2] = bounds.extents[2] + min[2];

	float r2 = 0;
	for (int i = 0; i < count; i++) {
		float offset[3];
		offset[0] = vertices[i].position[0] - bounds.origin[0];
		offset[1] = vertices[i].position[1] - bounds.origin[1];
		offset[2] = vertices[i].position[2] - bounds.origin[2];

		float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
		r2 = std::max(r2, distance);
	}

	bounds.radius = std::sqrt(r2);

	return bounds;
}