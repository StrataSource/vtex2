
#include <vector>
#include <cassert>

#include "enums.hpp"
#include "util.hpp"
#include "strtools.hpp"

VTFImageFormat ImageFormatFromString(const char* arg) {
	if (str::strcasecmp("IMAGE_FORMAT_RGBA8888", arg) == 0)
		return IMAGE_FORMAT_RGBA8888;
	if (str::strcasecmp("IMAGE_FORMAT_ABGR8888", arg) == 0)
		return IMAGE_FORMAT_ABGR8888;
	if (str::strcasecmp("IMAGE_FORMAT_RGB888", arg) == 0)
		return IMAGE_FORMAT_RGB888;
	if (str::strcasecmp("IMAGE_FORMAT_BGR888", arg) == 0)
		return IMAGE_FORMAT_BGR888;
	if (str::strcasecmp("IMAGE_FORMAT_RGB565", arg) == 0)
		return IMAGE_FORMAT_RGB565;
	if (str::strcasecmp("IMAGE_FORMAT_I8", arg) == 0)
		return IMAGE_FORMAT_I8;
	if (str::strcasecmp("IMAGE_FORMAT_IA88", arg) == 0)
		return IMAGE_FORMAT_IA88;
	if (str::strcasecmp("IMAGE_FORMAT_P8", arg) == 0)
		return IMAGE_FORMAT_P8;
	if (str::strcasecmp("IMAGE_FORMAT_A8", arg) == 0)
		return IMAGE_FORMAT_A8;
	if (str::strcasecmp("IMAGE_FORMAT_RGB888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_RGB888_BLUESCREEN;
	if (str::strcasecmp("IMAGE_FORMAT_BGR888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_BGR888_BLUESCREEN;
	if (str::strcasecmp("IMAGE_FORMAT_ARGB8888", arg) == 0)
		return IMAGE_FORMAT_ARGB8888;
	if (str::strcasecmp("IMAGE_FORMAT_BGRA8888", arg) == 0)
		return IMAGE_FORMAT_BGRA8888;
	if (str::strcasecmp("IMAGE_FORMAT_DXT1", arg) == 0)
		return IMAGE_FORMAT_DXT1;
	if (str::strcasecmp("IMAGE_FORMAT_DXT3", arg) == 0)
		return IMAGE_FORMAT_DXT3;
	if (str::strcasecmp("IMAGE_FORMAT_DXT5", arg) == 0)
		return IMAGE_FORMAT_DXT5;
	if (str::strcasecmp("IMAGE_FORMAT_BGRX8888", arg) == 0)
		return IMAGE_FORMAT_BGRX8888;
	if (str::strcasecmp("IMAGE_FORMAT_BGR565", arg) == 0)
		return IMAGE_FORMAT_BGR565;
	if (str::strcasecmp("IMAGE_FORMAT_BGRX5551", arg) == 0)
		return IMAGE_FORMAT_BGRX5551;
	if (str::strcasecmp("IMAGE_FORMAT_BGRA4444", arg) == 0)
		return IMAGE_FORMAT_BGRA4444;
	if (str::strcasecmp("IMAGE_FORMAT_DXT1_ONEBITALPHA", arg) == 0)
		return IMAGE_FORMAT_DXT1_ONEBITALPHA;
	if (str::strcasecmp("IMAGE_FORMAT_BGRA5551", arg) == 0)
		return IMAGE_FORMAT_BGRA5551;
	if (str::strcasecmp("IMAGE_FORMAT_UV88", arg) == 0)
		return IMAGE_FORMAT_UV88;
	if (str::strcasecmp("IMAGE_FORMAT_UVWQ8888", arg) == 0)
		return IMAGE_FORMAT_UVWQ8888;
	if (str::strcasecmp("IMAGE_FORMAT_RGBA16161616F", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616F;
	if (str::strcasecmp("IMAGE_FORMAT_RGBA16161616", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616;
	if (str::strcasecmp("IMAGE_FORMAT_UVLX8888", arg) == 0)
		return IMAGE_FORMAT_UVLX8888;
	if (str::strcasecmp("IMAGE_FORMAT_R32F", arg) == 0)
		return IMAGE_FORMAT_R32F;
	if (str::strcasecmp("IMAGE_FORMAT_RGB323232F", arg) == 0)
		return IMAGE_FORMAT_RGB323232F;
	if (str::strcasecmp("IMAGE_FORMAT_RGBA32323232F", arg) == 0)
		return IMAGE_FORMAT_RGBA32323232F;
	if (str::strcasecmp("IMAGE_FORMAT_NV_NULL", arg) == 0)
		return IMAGE_FORMAT_NV_NULL;
	if (str::strcasecmp("IMAGE_FORMAT_ATI2N", arg) == 0)
		return IMAGE_FORMAT_ATI2N;
	if (str::strcasecmp("IMAGE_FORMAT_ATI1N", arg) == 0)
		return IMAGE_FORMAT_ATI1N;
	if (str::strcasecmp("IMAGE_FORMAT_BC7", arg) == 0)
		return IMAGE_FORMAT_BC7;
	if (str::strcasecmp("IMAGE_FORMAT_COUNT", arg) == 0)
		return IMAGE_FORMAT_COUNT;
	return IMAGE_FORMAT_NONE;
}

std::vector<std::string> TextureFlagsToStringVector(std::uint32_t flags) {
	std::vector<std::string> ret;
	if (flags & TEXTUREFLAGS_POINTSAMPLE)
		ret.emplace_back("TEXTUREFLAGS_POINTSAMPLE");
	if (flags & TEXTUREFLAGS_TRILINEAR)
		ret.emplace_back("TEXTUREFLAGS_TRILINEAR");
	if (flags & TEXTUREFLAGS_CLAMPS)
		ret.emplace_back("TEXTUREFLAGS_CLAMPS");
	if (flags & TEXTUREFLAGS_CLAMPT)
		ret.emplace_back("TEXTUREFLAGS_CLAMPT");
	if (flags & TEXTUREFLAGS_ANISOTROPIC)
		ret.emplace_back("TEXTUREFLAGS_ANISOTROPIC");
	if (flags & TEXTUREFLAGS_HINT_DXT5)
		ret.emplace_back("TEXTUREFLAGS_HINT_DXT5");
	if (flags & TEXTUREFLAGS_SRGB)
		ret.emplace_back("TEXTUREFLAGS_SRGB");
	if (flags & TEXTUREFLAGS_DEPRECATED_NOCOMPRESS)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_NOCOMPRESS");
	if (flags & TEXTUREFLAGS_NORMAL)
		ret.emplace_back("TEXTUREFLAGS_NORMAL");
	if (flags & TEXTUREFLAGS_NOMIP)
		ret.emplace_back("TEXTUREFLAGS_NOMIP");
	if (flags & TEXTUREFLAGS_NOLOD)
		ret.emplace_back("TEXTUREFLAGS_NOLOD");
	if (flags & TEXTUREFLAGS_MINMIP)
		ret.emplace_back("TEXTUREFLAGS_MINMIP");
	if (flags & TEXTUREFLAGS_PROCEDURAL)
		ret.emplace_back("TEXTUREFLAGS_PROCEDURAL");
	if (flags & TEXTUREFLAGS_ONEBITALPHA)
		ret.emplace_back("TEXTUREFLAGS_ONEBITALPHA");
	if (flags & TEXTUREFLAGS_EIGHTBITALPHA)
		ret.emplace_back("TEXTUREFLAGS_EIGHTBITALPHA");
	if (flags & TEXTUREFLAGS_ENVMAP)
		ret.emplace_back("TEXTUREFLAGS_ENVMAP");
	if (flags & TEXTUREFLAGS_RENDERTARGET)
		ret.emplace_back("TEXTUREFLAGS_RENDERTARGET");
	if (flags & TEXTUREFLAGS_DEPTHRENDERTARGET)
		ret.emplace_back("TEXTUREFLAGS_DEPTHRENDERTARGET");
	if (flags & TEXTUREFLAGS_NODEBUGOVERRIDE)
		ret.emplace_back("TEXTUREFLAGS_NODEBUGOVERRIDE");
	if (flags & TEXTUREFLAGS_SINGLECOPY)
		ret.emplace_back("TEXTUREFLAGS_SINGLECOPY");
	if (flags & TEXTUREFLAGS_UNUSED0)
		ret.emplace_back("TEXTUREFLAGS_UNUSED0");
	if (flags & TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA");
	if (flags & TEXTUREFLAGS_UNUSED1)
		ret.emplace_back("TEXTUREFLAGS_UNUSED1");
	if (flags & TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL");
	if (flags & TEXTUREFLAGS_UNUSED2)
		ret.emplace_back("TEXTUREFLAGS_UNUSED2");
	if (flags & TEXTUREFLAGS_DEPRECATED_NORMALTODUDV)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_NORMALTODUDV");
	if (flags & TEXTUREFLAGS_UNUSED3)
		ret.emplace_back("TEXTUREFLAGS_UNUSED3");
	if (flags & TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION");
	if (flags & TEXTUREFLAGS_NODEPTHBUFFER)
		ret.emplace_back("TEXTUREFLAGS_NODEPTHBUFFER");
	if (flags & TEXTUREFLAGS_UNUSED4)
		ret.emplace_back("TEXTUREFLAGS_UNUSED4");
	if (flags & TEXTUREFLAGS_DEPRECATED_NICEFILTERED)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_NICEFILTERED");
	if (flags & TEXTUREFLAGS_CLAMPU)
		ret.emplace_back("TEXTUREFLAGS_CLAMPU");
	if (flags & TEXTUREFLAGS_VERTEXTEXTURE)
		ret.emplace_back("TEXTUREFLAGS_VERTEXTEXTURE");
	if (flags & TEXTUREFLAGS_SSBUMP)
		ret.emplace_back("TEXTUREFLAGS_SSBUMP");
	if (flags & TEXTUREFLAGS_UNUSED5)
		ret.emplace_back("TEXTUREFLAGS_UNUSED5");
	if (flags & TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK");
	if (flags & TEXTUREFLAGS_BORDER)
		ret.emplace_back("TEXTUREFLAGS_BORDER");
	if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_RED)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_SPECVAR_RED");
	if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA)
		ret.emplace_back("TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA");

	return ret;
}

const char* GetResourceName(vlUInt resource) {
	switch (resource) {
		case VTF_LEGACY_RSRC_LOW_RES_IMAGE:
			return "Low-res Image (Legacy)";
		case VTF_LEGACY_RSRC_IMAGE:
			return "Image (Legacy)";
		case VTF_RSRC_SHEET:
			return "Sheet";
		case VTF_RSRC_CRC:
			return "CRC";
		case VTF_RSRC_TEXTURE_LOD_SETTINGS:
			return "Texture LOD Settings";
		case VTF_RSRC_TEXTURE_SETTINGS_EX:
			return "Texture Settings Extended";
		case VTF_RSRC_KEY_VALUE_DATA:
			return "KeyValue Data";
		default:
			return "";
	}
}

VTFImageFormat ImageFormatFromUserString(const char* arg) {
	if (str::strcasecmp("RGBA8888", arg) == 0)
		return IMAGE_FORMAT_RGBA8888;
	if (str::strcasecmp("ABGR8888", arg) == 0)
		return IMAGE_FORMAT_ABGR8888;
	if (str::strcasecmp("RGB888", arg) == 0)
		return IMAGE_FORMAT_RGB888;
	if (str::strcasecmp("BGR888", arg) == 0)
		return IMAGE_FORMAT_BGR888;
	if (str::strcasecmp("RGB565", arg) == 0)
		return IMAGE_FORMAT_RGB565;
	if (str::strcasecmp("I8", arg) == 0)
		return IMAGE_FORMAT_I8;
	if (str::strcasecmp("IA88", arg) == 0)
		return IMAGE_FORMAT_IA88;
	if (str::strcasecmp("P8", arg) == 0)
		return IMAGE_FORMAT_P8;
	if (str::strcasecmp("A8", arg) == 0)
		return IMAGE_FORMAT_A8;
	if (str::strcasecmp("RGB888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_RGB888_BLUESCREEN;
	if (str::strcasecmp("BGR888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_BGR888_BLUESCREEN;
	if (str::strcasecmp("ARGB8888", arg) == 0)
		return IMAGE_FORMAT_ARGB8888;
	if (str::strcasecmp("BGRA8888", arg) == 0)
		return IMAGE_FORMAT_BGRA8888;
	if (str::strcasecmp("DXT1", arg) == 0)
		return IMAGE_FORMAT_DXT1;
	if (str::strcasecmp("DXT3", arg) == 0)
		return IMAGE_FORMAT_DXT3;
	if (str::strcasecmp("DXT5", arg) == 0)
		return IMAGE_FORMAT_DXT5;
	if (str::strcasecmp("BGRX8888", arg) == 0)
		return IMAGE_FORMAT_BGRX8888;
	if (str::strcasecmp("BGR565", arg) == 0)
		return IMAGE_FORMAT_BGR565;
	if (str::strcasecmp("BGRX5551", arg) == 0)
		return IMAGE_FORMAT_BGRX5551;
	if (str::strcasecmp("BGRA4444", arg) == 0)
		return IMAGE_FORMAT_BGRA4444;
	if (str::strcasecmp("DXT1_ONEBITALPHA", arg) == 0)
		return IMAGE_FORMAT_DXT1_ONEBITALPHA;
	if (str::strcasecmp("BGRA5551", arg) == 0)
		return IMAGE_FORMAT_BGRA5551;
	if (str::strcasecmp("UV88", arg) == 0)
		return IMAGE_FORMAT_UV88;
	if (str::strcasecmp("UVWQ8888", arg) == 0)
		return IMAGE_FORMAT_UVWQ8888;
	if (str::strcasecmp("RGBA16161616F", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616F;
	if (str::strcasecmp("RGBA16161616", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616;
	if (str::strcasecmp("UVLX8888", arg) == 0)
		return IMAGE_FORMAT_UVLX8888;
	if (str::strcasecmp("R32F", arg) == 0)
		return IMAGE_FORMAT_R32F;
	if (str::strcasecmp("RGB323232F", arg) == 0)
		return IMAGE_FORMAT_RGB323232F;
	if (str::strcasecmp("RGBA32323232F", arg) == 0)
		return IMAGE_FORMAT_RGBA32323232F;
	if (str::strcasecmp("NV_NULL", arg) == 0)
		return IMAGE_FORMAT_NV_NULL;
	if (str::strcasecmp("ATI2N", arg) == 0)
		return IMAGE_FORMAT_ATI2N;
	if (str::strcasecmp("ATI1N", arg) == 0)
		return IMAGE_FORMAT_ATI1N;
	if (str::strcasecmp("BC7", arg) == 0)
		return IMAGE_FORMAT_BC7;
	if (str::strcasecmp("COUNT", arg) == 0)
		return IMAGE_FORMAT_COUNT;
	return IMAGE_FORMAT_NONE;
}
