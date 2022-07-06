/**
 * Some utils - mainly for portability
 */
#pragma once

#include <cstring>
#include <fstream>
#include <string>
#include <memory>

#ifdef _WIN32
#	define strcasecmp _stricmp
#endif

/**
 * Helper to quickly read a file off disk
 */
static std::size_t read_file(const std::string& path, std::uint8_t*& outPtr) {
	std::ifstream stream(path, std::ios::in | std::ios::binary);
	outPtr = nullptr;
	if (!stream.good())
		return 0;

	stream.seekg(0, std::ios::end);
	auto size = stream.tellg();
	stream.seekg(0, std::ios::beg);
	
	outPtr = new std::uint8_t[size];
	stream.read((char*)outPtr, size);
	
	return size;
}
