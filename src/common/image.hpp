/**
 * imglib - Lightweight image library built on STB
 */
#pragma once

#include <filesystem>
#include <string>

#include "VTFLib.h"

namespace imglib
{

	/**
	 * Per-channel data type
	 */
	enum ChannelType {
		UInt8,
		UInt16,
		Float // Generally a linear FP number (32-BPC)
	};

	/**
	 * Image file format (On disk)
	 */
	enum FileFormat {
		None,
		Tga,
		Png,
		Jpeg,
		Bmp,
		Gif,
		Psd,
		Hdr
	};

	struct ImageInfo_t {
		int w, h;		  // Width/height
		int frames;		  // If animated images, num frames
		int comps;		  // Number of components (RGB=3, RGBA=4, etc)
		ChannelType type; // Per-channel type
	};

	struct ImageData_t {
		ImageInfo_t info;
		void* data; // Data is organized in w*h blocks, with each frame coming after one another
	};

	/**
	 * Load the image file
	 * returns a fp to the file, which is reused between other image_ funcs
	 * If this fails, it will return nullptr
	 */
	FILE* image_begin(const char* filePath);
	inline FILE* image_begin(const std::filesystem::path& path) {
		return image_begin(path.string().c_str());
	}

	/**
	 * Return image info for the FP
	 */
	ImageInfo_t image_info(FILE* fp);

	/**
	 * Load all image data
	 */
	ImageData_t image_load(FILE* fp);

	/**
	 * Frees image data
	 */
	void image_free(ImageData_t& data);

	/**
	 * Closes the fp and some internal cleanup
	 * Does nothing to any data loaded via image_load
	 */
	void image_end(FILE* fp);

	/**
	 * Saves the image to a file
	 * format is the requested file format
	 * Supported formats: tga, png, jpeg, bmp
	 */
	bool image_save(const ImageData_t& data, FILE* fp, FileFormat format);
	bool image_save(const ImageData_t& data, const char* file, FileFormat format);

	/**
	 * Helper to get an associated file type based on extension
	 * Returns FileFormat::None if not
	 */
	FileFormat image_get_format_from_file(const char* fileName);

	/**
	 * Returns image format for file extension
	 * The parameter here should be the file extension component AFTER the final .
	 * ie png, tga, psd
	 */
	FileFormat image_get_format(const char* str);

	/**
	 * Returns an extension (full extension, including .)
	 * for the specified image file type
	 */
	const char* image_get_extension(FileFormat format);

	/**
	 * Resizes an image in place
	 */
	bool resize(ImageData_t& data, int newW, int newH);

	/**
	 * Resize an image
	 * Variant for non-imglib data
	 */
	bool resize(void* data, void** outData, ChannelType type, int comps, int w, int h, int newW, int newH);

	/**
	 * Return the number of bytes needed for the specified image
	 */
	size_t bytes_for_image(int w, int h, ChannelType type, int comps);

	/**
	 * Apply processing effects to the image
	 * Right now this only handles GL -> DX transforms, but more processing flags may be added in the future
	 * Ideally all processing will be done in one go
	 */
	using ProcFlags = uint32_t;
	inline constexpr ProcFlags PROC_GL_TO_DX_NORM = (1<<0);

	bool process_image(void* data, ChannelType type, int comps, int w, int h, ProcFlags flags);
	inline bool process_image(ImageData_t& data, ProcFlags flags) {
		return process_image(data.data, data.info.type, data.info.comps, data.info.w, data.info.h, flags);
	} 


	/**
	 * Color conversion routines
	 * names are formatted as convert_srcFormat_dstFormat
	 * convert_formats can be used to convert between the 3 formats assuming they have the same
	 * component count. @TODO: remove this restriction!
	 */

	bool convert_formats(
		const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int comps, int w, int h);
	bool convert(ImageData_t& data, ChannelType dstChanType);

	template<int COMPS>
	void convert_16_to_8(const void* rgb16, void* rgb8, int w, int h) {
		const uint16_t* src = static_cast<const uint16_t*>(rgb16);
		uint8_t* dst = static_cast<uint8_t*>(rgb8);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * (255.f / 65535.f);
	}

	template<int COMPS>
	void convert_32_to_8(const void* rgb32, void* rgb8, int w, int h) {
		const float* src = static_cast<const float*>(rgb32);
		uint8_t* dst = static_cast<uint8_t*>(rgb8);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * (255.f);
	}

	template<int COMPS>
	void convert_8_to_32(const void* rgb8, void* rgb32, int w, int h) {
		const uint8_t* src = static_cast<const uint8_t*>(rgb8);
		float* dst = static_cast<float*>(rgb32);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] / (255.f);
	}

	template<int COMPS>
	void convert_16_to_32(const void* rgb16, void* rgb32, int w, int h) {
		const uint16_t* src = static_cast<const uint16_t*>(rgb16);
		float* dst = static_cast<float*>(rgb32);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] / (65535.f);
	}

	template<int COMPS>
	void convert_32_to_16(const void* rgb32, void* rgb16, int w, int h) {
		const float* src = static_cast<const float*>(rgb32);
		uint16_t* dst = static_cast<uint16_t*>(rgb16);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * 65535.f;
	}

	template<int COMPS>
	void convert_8_to_16(const void* rgb8, void* rgb16, int w, int h) {
		auto* src = static_cast<const uint8_t*>(rgb8);
		auto* dst = static_cast<uint16_t*>(rgb16);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * (65535.f / 255.f);
	}


	/**
	 * Get a compatible VTF image format for the image data
	 */
	VTFImageFormat get_vtf_format(const ImageInfo_t& info);

} // namespace imglib
