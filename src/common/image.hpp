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

	using ProcFlags = uint32_t;
	inline constexpr ProcFlags PROC_GL_TO_DX_NORM = (1 << 0);

	class Image {
	public:
		Image() = default;

		/**
		 * Given some data, create an image around it.
		 * @param wrapData If true, we will act as a wrapper around *data. Thus, *data MUST be valid for the lifetime of
		 * this object. if this is false, we'll malloc our own data and copy *data into that buffer
		 */
		Image(void* data, ChannelType type, int channels, int w, int h, bool wrapData = false);

		/**
		 * Creates an image with the data store required to store an image with the specified parameters
		 * @param clear if true, memset to 0
		 */
		Image(ChannelType type, int channels, int w, int h, bool clear = true);

		~Image();

		static inline std::shared_ptr<Image> load(const std::filesystem::path& path) {
			std::string s = path.string();
			return load(s.c_str());
		}

		/**
		 * Loads the image from the specified file
		 * Optionally FILE* can be specified directly
		 */
		static std::shared_ptr<Image> load(const char* file);
		static std::shared_ptr<Image> load(FILE* fp);

		/**
		 * @brief Clear internal data store, frees up some memory
		 */
		void clear();

		/**
		 * Apply processing effects to the image
		 * Right now this only handles GL -> DX transforms, but more processing flags may be added in the future
		 * Ideally all processing will be done in one go
		 */
		bool process(ProcFlags flags);

		/**
		 * Saves the image to a file
		 * format is the requested file format
		 * Supported formats: tga, png, jpeg, bmp
		 */
		bool save(const char* path, FileFormat format);

		/**
		 * Resize the image in-place
		 * @param w New width
		 * @param h New height
		 * @return True if the resize succeeded
		 */
		bool resize(int w, int h);

		/**
		 * Convert this image to the specified channel type
		 * Not all conversions are supported, so check the return value!
		 */
		bool convert(ChannelType type);

		/**
		 * Returns the VTF format which matches up to the data we have internally here
		 */
		VTFImageFormat vtf_format() const;

		int width() const {
			return m_width;
		}
		int height() const {
			return m_height;
		}
		int frames() const {
			return m_frames;
		}
		int channels() const {
			return m_comps;
		}
		ChannelType type() const {
			return m_type;
		}
		const void* data() const {
			return m_data;
		}

		void* data() {
			return m_data;
		}

		template <typename T>
		T* data() {
			return static_cast<T*>(m_data);
		}
		template <typename T>
		const T* data() const {
			return static_cast<T*>(m_data);
		}

	private:
		int m_width = 0;
		int m_height = 0;
		int m_frames = 0;	// If animated images, num frames
		int m_comps = 0;	// Number of components (RGB=3, RGBA=4, etc)
		ChannelType m_type; // Per-channel type
		FileFormat m_fileFmt = FileFormat::None;
		void* m_data = nullptr;
		bool m_owned = true;
	};

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
	 * Resize an image
	 * Variant for raw data
	 */
	bool resize(void* data, void** outData, ChannelType type, int channels, int w, int h, int newW, int newH);

	/**
	 * Return the number of bytes needed for the specified image
	 */
	size_t bytes_for_image(int w, int h, ChannelType type, int channels);

	/**
	 * Color conversion routines
	 * names are formatted as convert_srcFormat_dstFormat
	 * convert_formats can be used to convert between the 3 formats assuming they have the same
	 * component count. @TODO: remove this restriction!
	 */

	bool convert_formats(
		const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int comps, int w, int h);

	template <int COMPS>
	void convert_16_to_8(const void* rgb16, void* rgb8, int w, int h) {
		const uint16_t* src = static_cast<const uint16_t*>(rgb16);
		uint8_t* dst = static_cast<uint8_t*>(rgb8);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * (255.f / 65535.f);
	}

	template <int COMPS>
	void convert_32_to_8(const void* rgb32, void* rgb8, int w, int h) {
		const float* src = static_cast<const float*>(rgb32);
		uint8_t* dst = static_cast<uint8_t*>(rgb8);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * (255.f);
	}

	template <int COMPS>
	void convert_8_to_32(const void* rgb8, void* rgb32, int w, int h) {
		const uint8_t* src = static_cast<const uint8_t*>(rgb8);
		float* dst = static_cast<float*>(rgb32);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] / (255.f);
	}

	template <int COMPS>
	void convert_16_to_32(const void* rgb16, void* rgb32, int w, int h) {
		const uint16_t* src = static_cast<const uint16_t*>(rgb16);
		float* dst = static_cast<float*>(rgb32);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] / (65535.f);
	}

	template <int COMPS>
	void convert_32_to_16(const void* rgb32, void* rgb16, int w, int h) {
		const float* src = static_cast<const float*>(rgb32);
		uint16_t* dst = static_cast<uint16_t*>(rgb16);
		for (int i = 0; i < w * h * COMPS; ++i)
			dst[i] = src[i] * 65535.f;
	}

	template <int COMPS>
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
