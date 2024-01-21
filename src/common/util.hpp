/**
 * Some utils - mainly for portability
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <memory>
#include <charconv>

#include "strtools.hpp"

/******************************/
namespace util
{

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

		if (!size)
			return 0;

		outPtr = new std::uint8_t[size];
		stream.read((char*)outPtr, size);

		return size;
	}

	static inline bool strtoint(const std::string& str, int& out) {
		auto [p, err] = std::from_chars(str.c_str(), str.c_str() + str.length(), out);
		return err == std::errc();
	}

	/**
	 * RAII cleanup object
	 */
	template <class T>
	class cleanup {
	public:
		[[nodiscard]] cleanup(const T& obj)
			: m_func(obj) {
		}
		cleanup(const cleanup&) = delete;
		~cleanup() {
			m_func();
		}

	private:
		T m_func;
	};

	template <class T, std::size_t N>
	constexpr std::size_t ArraySize(T (&arr)[N]) {
		return N;
	}

	template <typename A, typename B, typename C>
	A clamp(const A& value, const B& min, const C& max) {
		return (value < min ? min : (value > max ? max : value));
	}

	static std::string& tolower(std::string& str) {
		for (int i = 0; i < str.size(); ++i)
			str[i] = std::tolower(str[i]);
		return str;
	}

	/*
	 * Get the last error which occurred in VTFLib
	 */
	const char* get_last_vtflib_error();
} // namespace util
