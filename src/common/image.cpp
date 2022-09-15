
#include "image.hpp"
#include "util.hpp"
#include "strtools.hpp"

#include <cstring>
#include <cassert>

// STB stuff
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

using namespace imglib;

FILE* imglib::image_begin(const char* filePath) {
	return fopen(filePath, "rb");
}

ImageInfo_t imglib::image_info(FILE* fp) {
	ImageInfo_t info;
	stbi_info_from_file(fp, &info.w, &info.h, &info.comps);
	info.frames = 1; // @TODO: animated image support

	// Determine channel type
	if (stbi_is_16_bit_from_file(fp))
		info.type = ChannelType::UInt16;
	else if (stbi_is_hdr_from_file(fp))
		info.type = ChannelType::Float;
	else
		info.type == ChannelType::UInt8;

	return info;
}

ImageData_t imglib::image_load(FILE* fp) {
	auto info = image_info(fp);
	ImageData_t data{};
	data.info.frames = 1;
	if (info.type == ChannelType::Float) {
		data.data = stbi_loadf_from_file(fp, &data.info.w, &data.info.h, &data.info.comps, info.comps);
	}
	else if (info.type == ChannelType::UInt16) {
		data.data = stbi_load_from_file_16(fp, &data.info.w, &data.info.h, &data.info.comps, info.comps);
	}
	else {
		data.data = stbi_load_from_file(fp, &data.info.w, &data.info.h, &data.info.comps, info.comps);
	}
	return data;
}

void imglib::image_free(ImageData_t& data) {
	free(data.data);
	data.data = nullptr;
}

void imglib::image_end(FILE* fp) {
	fclose(fp);
}

bool imglib::image_save(const ImageData_t& data, FILE* fp, FileFormat format) {
	if (!fp || !data.data || (format != Tga && format != Png && format != Jpeg && format != Bmp && format != Hdr))
		return false;

	if (format == Hdr) {
	}

	return true;
}

bool imglib::image_save(const ImageData_t& data, const char* file, FileFormat format) {
	if (!file || !data.data || (format != Tga && format != Png && format != Jpeg && format != Bmp && format != Hdr))
		return false;

	bool bOk = false;
	if (format == Hdr) {
		// Convert if necessary. Needs to be float for HDR
		auto* dataToUse = data.data;
		bool dataIsOurs = false;
		if (data.info.type != Float) {
			dataToUse = malloc(data.info.w * data.info.h * sizeof(float) * data.info.comps);
			dataIsOurs = true;
			if (!convert_formats(
					data.data, dataToUse, data.info.type, Float, data.info.comps, data.info.w, data.info.h)) {
				free(dataToUse);
				return false;
			}
		}

		bOk |= !!stbi_write_hdr(file, data.info.w, data.info.h, data.info.comps, (const float*)dataToUse);

		if (dataIsOurs)
			free(dataToUse);
	}
	else {
		// Convert to RGBX8 if not already in that format - required for the other writers
		auto* dataToUse = data.data;
		bool dataIsOurs = false;
		if (data.info.type != UInt8) {
			dataToUse = malloc(data.info.w * data.info.h * sizeof(uint8_t) * data.info.comps);
			dataIsOurs = true;
			if (!convert_formats(
					data.data, dataToUse, data.info.type, UInt8, data.info.comps, data.info.w, data.info.h)) {
				free(dataToUse);
				return false;
			}
		}

		// Write the stuff out
		if (format == Png) {
			bOk = !!stbi_write_png(file, data.info.w, data.info.h, data.info.comps, dataToUse, 0);
		}
		else if (format == Tga) {
			bOk = !!stbi_write_tga(file, data.info.w, data.info.h, data.info.comps, dataToUse);
		}
		else if (format == Jpeg) {
			bOk = !!stbi_write_jpg(file, data.info.w, data.info.h, data.info.comps, dataToUse, 100);
		}
		else if (format == Bmp) {
			bOk = !!stbi_write_bmp(file, data.info.w, data.info.h, data.info.comps, dataToUse);
		}

		if (dataIsOurs)
			free(dataToUse);
	}

	return bOk;
}

bool imglib::convert_formats(
	const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int comps, int w, int h) {
	// No conv needed
	if (srcChanType == dstChanType)
		return true;

	if (srcChanType == UInt8) {
		// RGBX32
		if (dstChanType == Float) {
			if (comps == 3) {
				convert_rgb8_rgb32(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba8_rgba32(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
		// RGBX16
		if (dstChanType == UInt16) {
			if (comps == 3) {
				convert_rgb8_rgb16(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba8_rgba16(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
	}
	// RGBX32 -> RGBX[8|16]
	else if (srcChanType == Float) {
		// RGBX16
		if (dstChanType == UInt16) {
			if (comps == 3) {
				convert_rgb32_rgb16(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba32_rgba16(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
		// RGBX8
		else if (dstChanType == UInt8) {
			if (comps == 3) {
				convert_rgb32_rgb8(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba32_rgba8(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
	}
	// RGBX16
	else if (srcChanType == UInt16) {
		if (dstChanType == UInt8) {
			if (comps == 3) {
				convert_rgb16_rgb8(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba16_rgba8(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
		else if (dstChanType == Float) {
			if (comps == 3) {
				convert_rgb16_rgb32(srcData, dstData, w, h);
				return true;
			}
			else if (comps == 4) {
				convert_rgba16_rgba32(srcData, dstData, w, h);
				return true;
			}
			else [[unlikely]]
				return false;
		}
	}

	return false;
}

void imglib::convert_rgb16_rgb8(const void* rgb16, void* rgb8, int w, int h) {
	const uint16_t* src = static_cast<const uint16_t*>(rgb16);
	uint8_t* dst = static_cast<uint8_t*>(rgb8);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] * (255.f / 65535.f);
}

void imglib::convert_rgba16_rgba8(const void* rgba16, void* rgba8, int w, int h) {
	const uint16_t* src = static_cast<const uint16_t*>(rgba16);
	uint8_t* dst = static_cast<uint8_t*>(rgba8);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] * (255.f / 65535.f);
}

void imglib::convert_rgb32_rgb8(const void* rgb32, void* rgb8, int w, int h) {
	const float* src = static_cast<const float*>(rgb32);
	uint8_t* dst = static_cast<uint8_t*>(rgb8);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] * (255.f);
}

void imglib::convert_rgba32_rgba8(const void* rgba32, void* rgba8, int w, int h) {
	const float* src = static_cast<const float*>(rgba32);
	uint8_t* dst = static_cast<uint8_t*>(rgba8);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] * (255.f);
}

void imglib::convert_rgb8_rgb32(const void* rgb8, void* rgb32, int w, int h) {
	const uint8_t* src = static_cast<const uint8_t*>(rgb8);
	float* dst = static_cast<float*>(rgb32);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] / (255.f);
}

void imglib::convert_rgba8_rgba32(const void* rgba8, void* rgba32, int w, int h) {
	const uint8_t* src = static_cast<const uint8_t*>(rgba8);
	float* dst = static_cast<float*>(rgba32);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] / (255.f);
}

void imglib::convert_rgb16_rgb32(const void* rgb16, void* rgb32, int w, int h) {
	const uint16_t* src = static_cast<const uint16_t*>(rgb16);
	float* dst = static_cast<float*>(rgb32);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] / (65535.f);
}

void imglib::convert_rgba16_rgba32(const void* rgba16, void* rgba32, int w, int h) {
	const uint16_t* src = static_cast<const uint16_t*>(rgba16);
	float* dst = static_cast<float*>(rgba32);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] / (65535.f);
}

void imglib::convert_rgb32_rgb16(const void* rgb32, void* rgb16, int w, int h) {
	const float* src = static_cast<const float*>(rgb32);
	uint16_t* dst = static_cast<uint16_t*>(rgb16);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] * 65535.f;
}

void imglib::convert_rgba32_rgba16(const void* rgba32, void* rgba16, int w, int h) {
	const float* src = static_cast<const float*>(rgba32);
	uint16_t* dst = static_cast<uint16_t*>(rgba16);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] * 65535.f;
}

void imglib::convert_rgb8_rgb16(const void* rgb8, void* rgb16, int w, int h) {
	auto* src = static_cast<const uint8_t*>(rgb8);
	auto* dst = static_cast<uint16_t*>(rgb16);
	for (int i = 0; i < w * h * 3; ++i)
		dst[i] = src[i] * (65535.f / 255.f);
}
void imglib::convert_rgba8_rgba16(const void* rgba8, void* rgba16, int w, int h) {
	auto* src = static_cast<const uint8_t*>(rgba8);
	auto* dst = static_cast<uint16_t*>(rgba16);
	for (int i = 0; i < w * h * 4; ++i)
		dst[i] = src[i] * (65535.f / 255.f);
}

FileFormat imglib::image_get_format_from_file(const char* str) {
	auto* ext = str::get_ext(str);
	return image_get_format(ext);
}

FileFormat imglib::image_get_format(const char* ext) {
	if (!str::strcasecmp(ext, "jpg") || !str::strcasecmp(ext, "jpeg"))
		return Jpeg;
	else if (!str::strcasecmp(ext, "png"))
		return Png;
	else if (!str::strcasecmp(ext, "tga"))
		return Tga;
	else if (!str::strcasecmp(ext, "bmp"))
		return Bmp;
	else if (!str::strcasecmp(ext, "gif"))
		return Gif;
	else if (!str::strcasecmp(ext, "psd"))
		return Psd;
	else if (!str::strcasecmp(ext, "hdr"))
		return Hdr;
	return FileFormat::None;
}

const char* imglib::image_get_extension(FileFormat format) {
	switch (format) {
		case Tga:
			return ".tga";
		case Png:
			return ".png";
		case Jpeg:
			return ".jpg";
		case Bmp:
			return ".bmp";
		case Gif:
			return ".gif";
		case Psd:
			return ".psd";
		case Hdr:
			return ".hdr";
		default:
			return "";
	}
}

VTFImageFormat imglib::get_vtf_format(const ImageInfo_t& info) {
	switch (info.type) {
		case imglib::UInt16:
			// @TODO: How to handle RGBA16? DONT i guess
			return IMAGE_FORMAT_RGBA16161616F;
		case imglib::Float:
			return (info.comps == 3) ? IMAGE_FORMAT_RGB323232F : IMAGE_FORMAT_RGBA32323232F;
		default:
			return (info.comps == 3) ? IMAGE_FORMAT_RGB888 : IMAGE_FORMAT_RGBA8888;
	}
}
