/**
 * imglib - Lightweight image library built on STB
 */
#pragma once

#include <filesystem>
#include <string>

#include "lwiconv.hpp"

#include "VTFLib.h"

namespace imglib
{

	constexpr int MAX_CHANNELS = 4;
	
	using lwiconv::make_swizzle;

	/**
	 * Per-channel data type
	 */
	enum class ChannelType {
		None = -1,
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
	inline constexpr ProcFlags PROC_INVERT_ALPHA = (1 << 1);
	
	uint32_t swizzle_from_str(const char* str);

	/**
	 * Returns the number of bytes per pixel for the format
	 */
	size_t pixel_size(ChannelType type, int channels);

	/**
	 * Returns the size of the channel type
	 */
	size_t channel_size(ChannelType type);

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

		static inline std::shared_ptr<Image> load(const std::filesystem::path& path, ChannelType convertOnLoad = ChannelType::None) {
			return load(path.string().c_str(), convertOnLoad);
		}

		/**
		 * Loads the image from the specified file
		 * Optionally FILE* can be specified directly
		 */
		static std::shared_ptr<Image> load(const char* file, ChannelType convertOnLoad = ChannelType::None);
		static std::shared_ptr<Image> load(FILE* fp, ChannelType convertOnLoad = ChannelType::None);

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
		 * @param type New channel type
		 * @param channels New channel count. If < 0, it is defaulted to m_comps
		 * @param pdef Default pixel fill for uninitialized pixels
		 */
		bool convert(ChannelType type, int channels = -1, const lwiconv::PixelF& pdef = {0,0,0,1});
		
		/**
		 * Perform in-place swizzle of components
		 * @param mask Swizzle mask. @see lwiconv::make_swizzle
		 */
		bool swizzle(uint32_t mask);

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

		size_t pixel_size() const {
			return imglib::pixel_size(m_type, m_comps);
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
		const void* srcData, void* dstData, ChannelType srcChanType, ChannelType dstChanType, int w, int h, int inComps, int outComps, int inStride, int outStride, const lwiconv::PixelF& pdefaults = {0, 0, 0, 1});

	/**
	 * Get a compatible VTF image format for the image data
	 */
	VTFImageFormat get_vtf_format(const ImageInfo_t& info);

} // namespace imglib
