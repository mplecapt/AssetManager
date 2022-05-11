#pragma once
#include "AssetLoader.h"

namespace SiegeEngine::Assets {

struct Vertex_f32_PNCV {
	float position[3];
	float normal[3];
	float color[3];
	float uv[3];
};

struct Vertex_P32N8C8V16 {
	float position[3];
	uint8_t normal[3];
	uint8_t color[3];
	float uv[2];
};

enum class VertexFormat : uint32_t {
	UNKNOWN = 0,
	PNCV_F32,
	P32N8C8V16
};

struct MeshBounds {
	float origin[3];
	float radius;
	float extents[3];
};
extern MeshBounds CalculateBounds(Vertex_f32_PNCV* vertices, size_t count);

struct MeshInfo {
	uint64_t vertexBufferSize;
	uint64_t indexBufferSize;
	MeshBounds bounds;
	VertexFormat vertexFormat;
	char indexSize;
	CompressionMode compressionMode;
	std::string originalFile;

	static MeshInfo Read(AssetFile* file);
	static void Unpack(MeshInfo* info, const char* srcbuffer, size_t srcsize, char* vertexBuffer, char* indexBuffer);
	static AssetFile Pack(MeshInfo* info, char* vertexData, char* indexData);
};

}