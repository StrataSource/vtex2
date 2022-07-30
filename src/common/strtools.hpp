/**
 * strtools.hpp - Common string utilities for C/C++
 */
#pragma once

#include <cstring>

namespace str
{

	static inline char* strncpy(char* dst, const char* src, size_t count) {
		auto ret = std::strncpy(dst, src, count);
		dst[count - 1] = 0;
		return ret;
	}

	template <size_t N>
	static inline char* strncpy(char (&dst)[N], const char* src) {
		return strncpy(dst, src, N);
	}

	/**
	 * Returns the file extension for str
	 * If no extension could be found, returns ""
	 * The extension is the part of the file name after the last / and .
	 * examples:
	 * 	some/path/file.exe -> exe
	 *  some/thing/file.sw.vtx -> sw.vtx
	 *  file -> (empty string)
	 *  file.mdl -> mdl
	 */
	static inline const char* get_ext(const char* str) {
		const char* e = nullptr;
		for (const char* s = str; s && *s; ++s) {
			if (*s == '/' || *s == '\\')
				e = nullptr;
			if (*s == '.' && !e)
				e = s + 1;
		}
		return e ? e : "";
	}

	static inline int strcasecmp(const char* s1, const char* s2) {
#ifdef _MSC_VER // if BAD_COMPILER
		return _stricmp(s1, s2);
#else // if GOOD_COMPILER
		return ::strcasecmp(s1, s2);
#endif
	}

	/**
	 * Return pointer to the component of the path after the last path sep
	 * some/path.vtf -> path.vtf
	 * path.vtf -> path.vtf
	 */
	static inline const char* get_filename(const char* str) {
		const char* lastSep = nullptr;

		for (const char* s = str; s && *s; ++s) {
			if (*s == '/' || *s == '\\')
				lastSep = s;
		}

		return lastSep ? ++lastSep : str;
	}

} // namespace str
