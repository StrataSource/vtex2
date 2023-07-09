/**
 * Pack.hpp - Simple channel packing interface
 *
 * Limitations:
 *  - For simplicity's sake, we only support RGB/A 888/8 targets
 *  - All source images must be the same size
 *
 */

#include <filesystem>

#include "VTFLib.h"

#include "image.hpp"

namespace pack
{

	struct ChannelPack_t {
		int srcChan;			// Src channel index (0=R, 1=G, 2=B, 3=A)
		int dstChan;			// Dest channel index (0=R, 1=G, 2=B, 3=A)
		const uint8_t* srcData; // RGBA8888 data
		int comps;				// Number of source components
		float constant;			// If srcData is nullptr, use this float constant (converted to RGBA8888)
	};

	/**
	 * Channel pack an image
	 * Requires all input image data be RGBA, and all input images be the same size
	 * w and h must also match the input image sizes
	 * destChannels is the number of channels in the output image
	 */
	std::shared_ptr<imglib::Image> pack_image(int destChannels, ChannelPack_t* channels, int numChannels, int w, int h);

} // namespace pack
