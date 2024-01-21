#include "VTFLib.h"

#include "util.hpp"

namespace util
{
	const char* get_last_vtflib_error() {
		auto error = vlGetLastError();
		if (error) {
			// +7 so we do not print `Error:\n`, which destroys the formatting
			return error + 7;
		}

		return "Unknown error";
	}
} // namespace util
