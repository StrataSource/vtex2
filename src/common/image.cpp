
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

static ImageInfo_t image_info(FILE* fp);

inline void* imgalloc(ChannelType type, int channels, int w, int h) {
	return malloc(imglib::bytes_for_image(w, h, type, channels));
}

Image::Image(void* data, ChannelType type, int channels, int w, int h, bool wrap)
	: m_width(w),
	  m_height(h),
	  m_type(type),
	  m_comps(channels) {
	if (wrap) {
		m_data = data;
		m_owned = false;
	}
	else {
		m_data = imgalloc(type, channels, w, h);
		memcpy(m_data, data, imglib::bytes_for_image(w, h, type, channels));
	}
}

Image::Image(ChannelType type, int channels, int w, int h, bool clear)
	: m_width(w),
	  m_height(h),
	  m_type(type),
	  m_comps(channels) {
	const auto size = imglib::bytes_for_image(w, h, type, channels);
	m_data = malloc(size);
	if (clear)
		memset(m_data, 0, size);
}

Image::~Image() {
	if (m_owned)
		free(m_data);
}

std::shared_ptr<Image> Image::load(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (!fp)
		return nullptr;
	auto img = load(fp);
	fclose(fp);
	return img;
}

std::shared_ptr<Image> Image::load(FILE* fp) {
	auto info = image_info(fp);
	auto image = std::make_shared<Image>();
	if (info.type == ChannelType::Float) {
		image->m_data = stbi_loadf_from_file(fp, &image->m_width, &image->m_height, &image->m_comps, 0);
	}
	else if (info.type == ChannelType::UInt16) {
		image->m_data = stbi_load_from_file_16(fp, &image->m_width, &image->m_height, &image->m_comps, 0);
	}
	else {
		image->m_data = stbi_load_from_file(fp, &image->m_width, &image->m_height, &image->m_comps, info.comps);
	}

	if (!image->m_data)
		return nullptr;
	return image;
}

void Image::clear() {
	if (m_owned)
		free(m_data);
	m_data = nullptr;
}

bool Image::save(const char* file, FileFormat format) {
	if (!file || !m_data || (format != Tga && format != Png && format != Jpeg && format != Bmp && format != Hdr))
		return false;

	bool bOk = false;
	if (format == Hdr) {
		// Convert if necessary. Needs to be float for HDR
		auto* dataToUse = m_data;
		bool dataIsOurs = false;
		if (m_type != Float) {
			dataToUse = malloc(m_width * m_height * sizeof(float) * m_comps);
			dataIsOurs = true;
			if (!convert_formats(m_data, dataToUse, m_type, Float, m_comps, m_width, m_height)) {
				free(dataToUse);
				return false;
			}
		}

		bOk |= !!stbi_write_hdr(file, m_width, m_height, m_comps, (const float*)dataToUse);

		if (dataIsOurs)
			free(dataToUse);
	}
	else {
		// Convert to RGBX8 if not already in that format - required for the other writers
		auto* dataToUse = m_data;
		bool dataIsOurs = false;
		if (m_type != UInt8) {
			dataToUse = malloc(m_width * m_height * sizeof(uint8_t) * m_comps);
			dataIsOurs = true;
			if (!convert_formats(m_data, dataToUse, m_type, UInt8, m_comps, m_width, m_height)) {
				free(dataToUse);
				return false;
			}
		}

		// Write the stuff out
		if (format == Png) {
			bOk = !!stbi_write_png(file, m_width, m_height, m_comps, dataToUse, 0);
		}
		else if (format == Tga) {
			bOk = !!stbi_write_tga(file, m_width, m_height, m_comps, dataToUse);
		}
		else if (format == Jpeg) {
			bOk = !!stbi_write_jpg(file, m_width, m_height, m_comps, dataToUse, 100);
		}
		else if (format == Bmp) {
			bOk = !!stbi_write_bmp(file, m_width, m_height, m_comps, dataToUse);
		}

		if (dataIsOurs)
			free(dataToUse);
	}

	return bOk;
}

bool Image::resize(int newW, int newH) {
	void* newData = nullptr;
	if (!imglib::resize(m_data, &newData, m_type, m_comps, m_width, m_height, newW, newH))
		return false;

	// Free old data
	free(m_data);
	m_data = newData;
	m_width = newW;
	m_height = newH;
	return true;
}

VTFImageFormat Image::vtf_format() const {
	switch (m_type) {
		case imglib::UInt16:
			// @TODO: How to handle RGBA16? DONT i guess
			return IMAGE_FORMAT_RGBA16161616F;
		case imglib::Float:
			return (m_comps == 3) ? IMAGE_FORMAT_RGB323232F
								  : (m_comps == 1 ? IMAGE_FORMAT_R32F : IMAGE_FORMAT_RGBA32323232F);
		default:
			return (m_comps == 3) ? IMAGE_FORMAT_RGB888 : (m_comps == 1 ? IMAGE_FORMAT_I8 : IMAGE_FORMAT_RGBA8888);
	}
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

template <int COMPS>
bool convert_formats_internal(
	const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int w, int h) {
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

bool Image::convert(ChannelType dstChanType) {
	void* dst = malloc(imglib::bytes_for_image(m_width, m_height, m_type, m_comps));

	if (!convert_formats(m_data, dst, m_type, dstChanType, m_comps, m_width, m_height)) {
		free(dst);
		return false;
	}

	free(m_data);
	m_data = dst;
	return true;
}

template <class T>
constexpr T FULL_VAL;
template <>
constexpr float FULL_VAL<float> = 1.0f;
template <>
constexpr uint16_t FULL_VAL<uint16_t> = UINT16_MAX;
template <>
constexpr uint8_t FULL_VAL<uint8_t> = UINT8_MAX;

template <class T>
static bool process_image_internal(void* indata, int comps, int w, int h, ProcFlags flags) {
	T* data = static_cast<T*>(indata);
	for (int i = 0; i < w * h * comps; i += comps) {
		T* cur = data + i;
		if (flags & PROC_GL_TO_DX_NORM)
			cur[1] = FULL_VAL<T> - cur[1]; // Invert green channel
	}
	return true;
}

bool Image::process(ProcFlags flags) {
	switch (m_type) {
		case UInt8:
			return process_image_internal<uint8_t>(m_data, m_comps, m_width, m_height, flags);
		case UInt16:
			return process_image_internal<uint16_t>(m_data, m_comps, m_width, m_height, flags);
		case Float:
			return process_image_internal<float>(m_data, m_comps, m_width, m_height, flags);
		default:
			assert(0);
	}
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

static ImageInfo_t image_info(FILE* fp) {
	ImageInfo_t info{};
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
