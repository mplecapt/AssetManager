#include "AssetLoader.h"
#include <fstream>

using namespace SiegeEngine::Assets;

bool AssetFile::Save(const char* path, const AssetFile& file) {
	std:: ofstream outfile;
	outfile.open(path, std::ios::binary | std::ios::out);

	// write type
	outfile.write(file.type, 4);
	// write version
	uint32_t version = file.version;
	outfile.write((const char*)&version, sizeof(uint32_t));
	// write json length
	uint32_t length = static_cast<uint32_t>(file.json.size());
	outfile.write((const char*)&length, sizeof(uint32_t));
	// write blob length
	uint32_t blobLength = static_cast<uint32_t>(file.binaryBlob.size());
	outfile.write((const char*)&blobLength, sizeof(uint32_t));
	// write json and blob data
	outfile.write(file.json.data(), length);
	outfile.write(file.binaryBlob.data(), blobLength);
	
	outfile.close();
	return true;
}

bool AssetFile::Load(const char* path, AssetFile& outputFile) {
	std::ifstream infile;
	infile.open(path, std::ios::binary);
	if (!infile.is_open()) return false;
	infile.seekg(0);

	// read type
	infile.read(outputFile.type, 4);
	// read version
	infile.read((char*)&outputFile.version, sizeof(uint32_t));
	// read json length
	uint32_t jsonlen = 0;
	infile.read((char*)&jsonlen, sizeof(uint32_t));
	// read blob length
	uint32_t bloblen = 0;
	infile.read((char*)&bloblen, sizeof(uint32_t));
	// read json data
	outputFile.json.resize(jsonlen);
	infile.read(outputFile.json.data(), jsonlen);
	// read blob data
	outputFile.binaryBlob.resize(bloblen);
	infile.read(outputFile.binaryBlob.data(), bloblen);

	return true;
}

CompressionMode SiegeEngine::Assets::ParseCompression(const char* f) {
	if (strcmp(f, "LZ4") == 0)
		return CompressionMode::LZ4;
	else
		return CompressionMode::NONE;
}