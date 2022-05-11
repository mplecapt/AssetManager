#include <iostream>
#include <filesystem>
#include <conio.h>

#include <AssetLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <AssetTexture.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <AssetMesh.h>

using namespace SiegeEngine::Assets;
namespace fs = std::filesystem;

void ConvertDir(const fs::path& dir);
bool ConvertTexture(const fs::path& input, const fs::path& output);
bool ConvertMesh(const fs::path& input, const fs::path& output);

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "\033[;31m" << "Too few arguments." << "\033[0m" << std::endl
			<< "Usage: AssetManager.exe [path_to_asset_dir]" << std::endl;
		return -1;
	}

	ConvertDir(fs::path{ argv[1] });

	std::cout << "Done. Press any key to continue..." << std::endl;
	_getch();
	return 0;
}

/**
 * @brief Converts each file in directory to an asset file
 * 
 * @param dir 
 */
#define printGrn(txt) "\033[32m" << txt << "\033[0m"
#define printYel(txt) "\033[33m" << txt << "\033[0m"
void ConvertDir(const fs::path& dir) {
	std::cout << "\033[42m" << "Loading asset directory:" << "\033[0m " << dir << std::endl;

	for (auto& p : fs::directory_iterator(dir)) {
		std::cout << printGrn("File: ") << p;

		if (p.path().extension() == ".png") {
			std::cout << printGrn("\t\tTexture") << std::endl;

			auto newpath = p.path();
			newpath.replace_extension(".tx");
			ConvertTexture(p.path(), newpath);
		} else if (p.path().extension() == ".obj") {
			std::cout << printGrn("\t\tMesh") << std::endl;

			auto newpath = p.path();
			newpath.replace_extension(".mesh");
			ConvertMesh(p.path(), newpath);
		} else {
			std::cout << printYel("\t\tSkipped") << std::endl;
		}
	}
}

/**
 * @brief Converts image file to texture asset
 * Currently uses stb_image
 * 
 * @param input png image filepath
 * @param output tx asset filepath
 * @return true successfully saved texture file
 * @return false failed to process file
 */
bool ConvertTexture(const fs::path& input, const fs::path& output) {
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(input.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "\033[31mFailed to load texture file: " << input << "\033[0m" << std::endl;
		return false;
	}

	int texture_size = texWidth * texHeight * 4;

	TextureInfo texinfo;
	texinfo.textureSize = texture_size;
	texinfo.pixelsize[0] = texWidth;
	texinfo.pixelsize[1] = texHeight;
	texinfo.textureFormat = TextureFormat::RGBA8;
	texinfo.originalFile = input.string();

	AssetFile newImage = TextureInfo::Pack(&texinfo, pixels);

	stbi_image_free(pixels);
	
	if (AssetFile::Save(output.string().c_str(), newImage)) {
		return true;
	} else {
		std::cout << "\033[31mFailed to save asset file: " << output << "\033[0m" << std::endl;
		return false;
	}
}

void PackVertex(SiegeEngine::Assets::Vertex_f32_PNCV new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy) {
	new_vert.position[0] = vx;
	new_vert.position[1] = vy;
	new_vert.position[2] = vz;

	new_vert.normal[0] = nx;
	new_vert.normal[1] = ny;
	new_vert.normal[2] = nz;

	new_vert.uv[0] = ux;
	new_vert.uv[1] = 1 - uy;
}

void PackVertex(SiegeEngine::Assets::Vertex_P32N8C8V16 new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy) {
	new_vert.position[0] = vx;
	new_vert.position[1] = vy;
	new_vert.position[2] = vz;

	new_vert.normal[0] = uint8_t( ((nx + 1.0) / 2.0) * 255 );
	new_vert.normal[1] = uint8_t( ((ny + 1.0) / 2.0) * 255 );
	new_vert.normal[2] = uint8_t( ((nz + 1.0) / 2.0) * 255 );

	new_vert.uv[0] = ux;
	new_vert.uv[1] = 1 - uy;
}

template<typename V>
void ExtractMeshFromObj(std::vector<tinyobj::shape_t>& shapes, tinyobj::attrib_t& attrib, std::vector<uint32_t>& _indices, std::vector<V>& _vertices) {
	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = 3;
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

				tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];

				V new_vert;
				PackVertex(new_vert, vx, vy, vz, nx, ny, nz, ux, uy);

				_indices.push_back(_vertices.size());
				_vertices.push_back(new_vert);
			}
			index_offset += fv;
		}
	}
}

/**
 * @brief Converts obj file to mesh asset file
 * 
 * @param input obj file path
 * @param output mesh file path
 * @return true successfully saved mesh file
 * @return false failed to load obj or save mesh files
 */
bool ConvertMesh(const fs::path& input, const fs::path& output) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, input.string().c_str(), nullptr);

	if (!warn.empty())
		std::cout << "\033[33mWARN: " << warn << "\033[0m" << std::endl;

	if (!err.empty()) {
		std::cerr << "\033[31mERR: " << err << "\033[0m" << std::endl;
		return false;
	}

	using VertexFormat = SiegeEngine::Assets::Vertex_f32_PNCV;
	auto VertexFormatEnum = SiegeEngine::Assets::VertexFormat::PNCV_F32;

	std::vector<VertexFormat> _vertices;
	std::vector<uint32_t> _indices;
	
	ExtractMeshFromObj(shapes, attrib, _indices, _vertices);


	MeshInfo meshinfo;
	meshinfo.vertexFormat = VertexFormatEnum;
	meshinfo.vertexBufferSize = _vertices.size() * sizeof(VertexFormat);
	meshinfo.indexBufferSize = _indices.size() * sizeof(uint32_t);
	meshinfo.indexSize = sizeof(uint32_t);
	meshinfo.originalFile = input.string();
	meshinfo.bounds = SiegeEngine::Assets::CalculateBounds(_vertices.data(), _vertices.size());
	
	AssetFile newFile = MeshInfo::Pack(&meshinfo, (char*)_vertices.data(), (char*)_indices.data());

	if (AssetFile::Save(output.string().c_str(), newFile)) {
		return true;
	} else {
		std::cout << "\033[31mFailed to save asset file: " << output << "\033[0m" << std::endl;
		return false;
	}
}