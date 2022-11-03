
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
#include "stb/stb_image_resize.h"

using namespace imglib;

FILE* imglib::image_begin(const char* filePath) {
	return fopen(filePath, "rb");
}

ImageInfo_t imglib::image_info(FILE* fp) {
	ImageInfo_t info {};
	stbi_info_from_file(fp, &info.w, &info.h, &info.comps);
	info.frames = 1; // @TODO: animated image support

	// Determine channel type
	if (stbi_is_16_bit_from_file(fp))
		info.type = ChannelType::UInt16;
	else if (stbi_is_hdr_from_file(fp))
		info.type = ChannelType::Float;
	else
		info.type = ChannelType::UInt8;

	return info;
}

ImageData_t imglib::image_load(FILE* fp) {
	auto info = image_info(fp);
	ImageData_t data{};
	data.info = info;
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
	if (!data.data)
		return;
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

bool imglib::resize(ImageData_t& data, int newW, int newH) {
	void* newData = nullptr;
	bool ret = resize(data.data, &newData, data.info.type, data.info.comps, data.info.w, data.info.h, newW, newH);

	if (!ret)
		return false;

	// Free old data
	free(data.data);
	data.data = newData;
	data.info.w = newW;
	data.info.h = newH;
	return true;
}

bool imglib::resize(
	void* indata, void** useroutdata, ChannelType srcType, int comps, int w, int h, int newW, int newH) {
	stbir_datatype type;
	switch (srcType) {
		case Float:
			type = STBIR_TYPE_FLOAT;
			break;
		case UInt16:
			type = STBIR_TYPE_UINT16;
			break;
		default:
			type = STBIR_TYPE_UINT8;
			break;
	}

	void* outdata = malloc(bytes_for_image(newW, newH, srcType, comps));

	int ret = stbir_resize(
		indata, w, h, 0, outdata, newW, newH, 0, type, comps, comps > 3, STBIR_FLAG_ALPHA_PREMULTIPLIED,
		STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR,
		nullptr);
	// Error :(
	if (!ret) {
		free(outdata);
		return false;
	}

	*useroutdata = outdata;
	return true;
}

size_t imglib::bytes_for_image(int w, int h, ChannelType type, int comps) {
	int bpc = 1;
	switch (type) {
		case ChannelType::Float:
			bpc = 4;
			break;
		case ChannelType::UInt16:
			bpc = 2;
			break;
		default:
			bpc = 1;
			break;
	}
	return w * h * comps * bpc;
}

template<int COMPS>
bool convert_formats_internal(const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int w, int h) {
	static_assert(COMPS > 0 && COMPS <= 4, "Comps is out of range");

	if (srcChanType == UInt8) {
		// RGBX32
		if (dstChanType == Float)
			convert_8_to_32<COMPS>(srcData, dstData, w, h);
		// RGBX16
		else if (dstChanType == UInt16)
			convert_8_to_16<COMPS>(srcData, dstData, w, h);
		return true;
	}
	// RGBX32 -> RGBX[8|16]
	else if (srcChanType == Float) {
		// RGBX16
		if (dstChanType == UInt16)
			convert_32_to_16<COMPS>(srcData, dstData, w, h);
		// RGBX8
		else if (dstChanType == UInt8)
			convert_32_to_8<COMPS>(srcData, dstData, w, h);
		return true;
	}
	// RGBX16
	else if (srcChanType == UInt16) {
		if (dstChanType == UInt8)
			convert_16_to_8<COMPS>(srcData, dstData, w, h);
		else if (dstChanType == Float)
			convert_16_to_32<COMPS>(srcData, dstData, w, h);
		return true;
	}

	return false;
}

bool imglib::convert_formats(
	const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int comps, int w, int h) {
	// No conv needed
	if (srcChanType == dstChanType)
		return true;

	if (comps == 4)
		return convert_formats_internal<4>(srcData, dstData, srcChanType, dstChanType, w, h);
	else if (comps == 3)
		return convert_formats_internal<3>(srcData, dstData, srcChanType, dstChanType, w, h);
	else if (comps == 2)
		return convert_formats_internal<2>(srcData, dstData, srcChanType, dstChanType, w, h);
	else if (comps == 1)
		return convert_formats_internal<1>(srcData, dstData, srcChanType, dstChanType, w, h);
	return false;
}

bool imglib::convert(ImageData_t& data, ChannelType dstChanType) {
	void* dst = malloc(imglib::bytes_for_image(data.info.w, data.info.h, data.info.type, data.info.comps));

	if (!convert_formats(data.data, dst, data.info.type, dstChanType, data.info.comps, data.info.w, data.info.h)) {
		free(dst);
		return false;
	}

	free(data.data);
	data.data = dst;
	return true;
}

template<typename T>
static T fullval() = delete;
template<> float fullval<float>() { return 1.0f; }
template<> uint16_t fullval<uint16_t>() { return 65535; }
template<> uint8_t fullval<uint8_t>() { return 255; }

template<class T>
static bool process_image_internal(void* indata, int comps, int w, int h, ProcFlags flags) {
	T* data = static_cast<T*>(indata);
	for (int i = 0; i < w * h * comps; i += comps) {
		T* cur = data + i;
		if (flags & PROC_GL_TO_DX_NORM)
			cur[1] = fullval<T>()-cur[1]; // Invert green channel
	}
	return true;
}

bool imglib::process_image(void* data, ChannelType type, int comps, int w, int h, ProcFlags flags) {
	if (type == UInt8)
		return process_image_internal<uint8_t>(data, comps, w, h, flags);
	else if (type == UInt16)
		return process_image_internal<uint16_t>(data, comps, w, h, flags);
	else if (type == Float)
		return process_image_internal<float>(data, comps, w, h, flags);
	return false;
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
