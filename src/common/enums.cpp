
#include <vector>
#include <cassert>

#include "enums.hpp"
#include "util.hpp"
#include "strtools.hpp"

VTFImageFormat ImageFormatFromString(const char* arg) {
	if (str::strcasecmp("IMAGE_FORMAT_RGBA8888", arg) == 0)
		return IMAGE_FORMAT_RGBA8888;
	else if (str::strcasecmp("IMAGE_FORMAT_ABGR8888", arg) == 0)
		return IMAGE_FORMAT_ABGR8888;
	else if (str::strcasecmp("IMAGE_FORMAT_RGB888", arg) == 0)
		return IMAGE_FORMAT_RGB888;
	else if (str::strcasecmp("IMAGE_FORMAT_BGR888", arg) == 0)
		return IMAGE_FORMAT_BGR888;
	else if (str::strcasecmp("IMAGE_FORMAT_RGB565", arg) == 0)
		return IMAGE_FORMAT_RGB565;
	else if (str::strcasecmp("IMAGE_FORMAT_I8", arg) == 0)
		return IMAGE_FORMAT_I8;
	else if (str::strcasecmp("IMAGE_FORMAT_IA88", arg) == 0)
		return IMAGE_FORMAT_IA88;
	else if (str::strcasecmp("IMAGE_FORMAT_P8", arg) == 0)
		return IMAGE_FORMAT_P8;
	else if (str::strcasecmp("IMAGE_FORMAT_A8", arg) == 0)
		return IMAGE_FORMAT_A8;
	else if (str::strcasecmp("IMAGE_FORMAT_RGB888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_RGB888_BLUESCREEN;
	else if (str::strcasecmp("IMAGE_FORMAT_BGR888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_BGR888_BLUESCREEN;
	else if (str::strcasecmp("IMAGE_FORMAT_ARGB8888", arg) == 0)
		return IMAGE_FORMAT_ARGB8888;
	else if (str::strcasecmp("IMAGE_FORMAT_BGRA8888", arg) == 0)
		return IMAGE_FORMAT_BGRA8888;
	else if (str::strcasecmp("IMAGE_FORMAT_DXT1", arg) == 0)
		return IMAGE_FORMAT_DXT1;
	else if (str::strcasecmp("IMAGE_FORMAT_DXT3", arg) == 0)
		return IMAGE_FORMAT_DXT3;
	else if (str::strcasecmp("IMAGE_FORMAT_DXT5", arg) == 0)
		return IMAGE_FORMAT_DXT5;
	else if (str::strcasecmp("IMAGE_FORMAT_BGRX8888", arg) == 0)
		return IMAGE_FORMAT_BGRX8888;
	else if (str::strcasecmp("IMAGE_FORMAT_BGR565", arg) == 0)
		return IMAGE_FORMAT_BGR565;
	else if (str::strcasecmp("IMAGE_FORMAT_BGRX5551", arg) == 0)
		return IMAGE_FORMAT_BGRX5551;
	else if (str::strcasecmp("IMAGE_FORMAT_BGRA4444", arg) == 0)
		return IMAGE_FORMAT_BGRA4444;
	else if (str::strcasecmp("IMAGE_FORMAT_DXT1_ONEBITALPHA", arg) == 0)
		return IMAGE_FORMAT_DXT1_ONEBITALPHA;
	else if (str::strcasecmp("IMAGE_FORMAT_BGRA5551", arg) == 0)
		return IMAGE_FORMAT_BGRA5551;
	else if (str::strcasecmp("IMAGE_FORMAT_UV88", arg) == 0)
		return IMAGE_FORMAT_UV88;
	else if (str::strcasecmp("IMAGE_FORMAT_UVWQ8888", arg) == 0)
		return IMAGE_FORMAT_UVWQ8888;
	else if (str::strcasecmp("IMAGE_FORMAT_RGBA16161616F", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616F;
	else if (str::strcasecmp("IMAGE_FORMAT_RGBA16161616", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616;
	else if (str::strcasecmp("IMAGE_FORMAT_UVLX8888", arg) == 0)
		return IMAGE_FORMAT_UVLX8888;
	else if (str::strcasecmp("IMAGE_FORMAT_R32F", arg) == 0)
		return IMAGE_FORMAT_R32F;
	else if (str::strcasecmp("IMAGE_FORMAT_RGB323232F", arg) == 0)
		return IMAGE_FORMAT_RGB323232F;
	else if (str::strcasecmp("IMAGE_FORMAT_RGBA32323232F", arg) == 0)
		return IMAGE_FORMAT_RGBA32323232F;
	else if (str::strcasecmp("IMAGE_FORMAT_NV_NULL", arg) == 0)
		return IMAGE_FORMAT_NV_NULL;
	else if (str::strcasecmp("IMAGE_FORMAT_ATI2N", arg) == 0)
		return IMAGE_FORMAT_ATI2N;
	else if (str::strcasecmp("IMAGE_FORMAT_ATI1N", arg) == 0)
		return IMAGE_FORMAT_ATI1N;
	else if (str::strcasecmp("IMAGE_FORMAT_BC7", arg) == 0)
		return IMAGE_FORMAT_BC7;
	else if (str::strcasecmp("IMAGE_FORMAT_COUNT", arg) == 0)
		return IMAGE_FORMAT_COUNT;
	else
		return IMAGE_FORMAT_NONE;
}

std::vector<std::string> TextureFlagsToStringVector(std::uint32_t flags) {
	std::vector<std::string> ret;
	if (flags & TEXTUREFLAGS_POINTSAMPLE)
		ret.push_back("TEXTUREFLAGS_POINTSAMPLE");
	if (flags & TEXTUREFLAGS_TRILINEAR)
		ret.push_back("TEXTUREFLAGS_TRILINEAR");
	if (flags & TEXTUREFLAGS_CLAMPS)
		ret.push_back("TEXTUREFLAGS_CLAMPS");
	if (flags & TEXTUREFLAGS_CLAMPT)
		ret.push_back("TEXTUREFLAGS_CLAMPT");
	if (flags & TEXTUREFLAGS_ANISOTROPIC)
		ret.push_back("TEXTUREFLAGS_ANISOTROPIC");
	if (flags & TEXTUREFLAGS_HINT_DXT5)
		ret.push_back("TEXTUREFLAGS_HINT_DXT5");
	if (flags & TEXTUREFLAGS_SRGB)
		ret.push_back("TEXTUREFLAGS_SRGB");
	if (flags & TEXTUREFLAGS_DEPRECATED_NOCOMPRESS)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_NOCOMPRESS");
	if (flags & TEXTUREFLAGS_NORMAL)
		ret.push_back("TEXTUREFLAGS_NORMAL");
	if (flags & TEXTUREFLAGS_NOMIP)
		ret.push_back("TEXTUREFLAGS_NOMIP");
	if (flags & TEXTUREFLAGS_NOLOD)
		ret.push_back("TEXTUREFLAGS_NOLOD");
	if (flags & TEXTUREFLAGS_MINMIP)
		ret.push_back("TEXTUREFLAGS_MINMIP");
	if (flags & TEXTUREFLAGS_PROCEDURAL)
		ret.push_back("TEXTUREFLAGS_PROCEDURAL");
	if (flags & TEXTUREFLAGS_ONEBITALPHA)
		ret.push_back("TEXTUREFLAGS_ONEBITALPHA");
	if (flags & TEXTUREFLAGS_EIGHTBITALPHA)
		ret.push_back("TEXTUREFLAGS_EIGHTBITALPHA");
	if (flags & TEXTUREFLAGS_ENVMAP)
		ret.push_back("TEXTUREFLAGS_ENVMAP");
	if (flags & TEXTUREFLAGS_RENDERTARGET)
		ret.push_back("TEXTUREFLAGS_RENDERTARGET");
	if (flags & TEXTUREFLAGS_DEPTHRENDERTARGET)
		ret.push_back("TEXTUREFLAGS_DEPTHRENDERTARGET");
	if (flags & TEXTUREFLAGS_NODEBUGOVERRIDE)
		ret.push_back("TEXTUREFLAGS_NODEBUGOVERRIDE");
	if (flags & TEXTUREFLAGS_SINGLECOPY)
		ret.push_back("TEXTUREFLAGS_SINGLECOPY");
	if (flags & TEXTUREFLAGS_UNUSED0)
		ret.push_back("TEXTUREFLAGS_UNUSED0");
	if (flags & TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_ONEOVERMIPLEVELINALPHA");
	if (flags & TEXTUREFLAGS_UNUSED1)
		ret.push_back("TEXTUREFLAGS_UNUSED1");
	if (flags & TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_PREMULTCOLORBYONEOVERMIPLEVEL");
	if (flags & TEXTUREFLAGS_UNUSED2)
		ret.push_back("TEXTUREFLAGS_UNUSED2");
	if (flags & TEXTUREFLAGS_DEPRECATED_NORMALTODUDV)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_NORMALTODUDV");
	if (flags & TEXTUREFLAGS_UNUSED3)
		ret.push_back("TEXTUREFLAGS_UNUSED3");
	if (flags & TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_ALPHATESTMIPGENERATION");
	if (flags & TEXTUREFLAGS_NODEPTHBUFFER)
		ret.push_back("TEXTUREFLAGS_NODEPTHBUFFER");
	if (flags & TEXTUREFLAGS_UNUSED4)
		ret.push_back("TEXTUREFLAGS_UNUSED4");
	if (flags & TEXTUREFLAGS_DEPRECATED_NICEFILTERED)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_NICEFILTERED");
	if (flags & TEXTUREFLAGS_CLAMPU)
		ret.push_back("TEXTUREFLAGS_CLAMPU");
	if (flags & TEXTUREFLAGS_VERTEXTEXTURE)
		ret.push_back("TEXTUREFLAGS_VERTEXTEXTURE");
	if (flags & TEXTUREFLAGS_SSBUMP)
		ret.push_back("TEXTUREFLAGS_SSBUMP");
	if (flags & TEXTUREFLAGS_UNUSED5)
		ret.push_back("TEXTUREFLAGS_UNUSED5");
	if (flags & TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_UNFILTERABLE_OK");
	if (flags & TEXTUREFLAGS_BORDER)
		ret.push_back("TEXTUREFLAGS_BORDER");
	if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_RED)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_SPECVAR_RED");
	if (flags & TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA)
		ret.push_back("TEXTUREFLAGS_DEPRECATED_SPECVAR_ALPHA");

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
	else if (str::strcasecmp("ABGR8888", arg) == 0)
		return IMAGE_FORMAT_ABGR8888;
	else if (str::strcasecmp("RGB888", arg) == 0)
		return IMAGE_FORMAT_RGB888;
	else if (str::strcasecmp("BGR888", arg) == 0)
		return IMAGE_FORMAT_BGR888;
	else if (str::strcasecmp("RGB565", arg) == 0)
		return IMAGE_FORMAT_RGB565;
	else if (str::strcasecmp("I8", arg) == 0)
		return IMAGE_FORMAT_I8;
	else if (str::strcasecmp("IA88", arg) == 0)
		return IMAGE_FORMAT_IA88;
	else if (str::strcasecmp("P8", arg) == 0)
		return IMAGE_FORMAT_P8;
	else if (str::strcasecmp("A8", arg) == 0)
		return IMAGE_FORMAT_A8;
	else if (str::strcasecmp("RGB888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_RGB888_BLUESCREEN;
	else if (str::strcasecmp("BGR888_BLUESCREEN", arg) == 0)
		return IMAGE_FORMAT_BGR888_BLUESCREEN;
	else if (str::strcasecmp("ARGB8888", arg) == 0)
		return IMAGE_FORMAT_ARGB8888;
	else if (str::strcasecmp("BGRA8888", arg) == 0)
		return IMAGE_FORMAT_BGRA8888;
	else if (str::strcasecmp("DXT1", arg) == 0)
		return IMAGE_FORMAT_DXT1;
	else if (str::strcasecmp("DXT3", arg) == 0)
		return IMAGE_FORMAT_DXT3;
	else if (str::strcasecmp("DXT5", arg) == 0)
		return IMAGE_FORMAT_DXT5;
	else if (str::strcasecmp("BGRX8888", arg) == 0)
		return IMAGE_FORMAT_BGRX8888;
	else if (str::strcasecmp("BGR565", arg) == 0)
		return IMAGE_FORMAT_BGR565;
	else if (str::strcasecmp("BGRX5551", arg) == 0)
		return IMAGE_FORMAT_BGRX5551;
	else if (str::strcasecmp("BGRA4444", arg) == 0)
		return IMAGE_FORMAT_BGRA4444;
	else if (str::strcasecmp("DXT1_ONEBITALPHA", arg) == 0)
		return IMAGE_FORMAT_DXT1_ONEBITALPHA;
	else if (str::strcasecmp("BGRA5551", arg) == 0)
		return IMAGE_FORMAT_BGRA5551;
	else if (str::strcasecmp("UV88", arg) == 0)
		return IMAGE_FORMAT_UV88;
	else if (str::strcasecmp("UVWQ8888", arg) == 0)
		return IMAGE_FORMAT_UVWQ8888;
	else if (str::strcasecmp("RGBA16161616F", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616F;
	else if (str::strcasecmp("RGBA16161616", arg) == 0)
		return IMAGE_FORMAT_RGBA16161616;
	else if (str::strcasecmp("UVLX8888", arg) == 0)
		return IMAGE_FORMAT_UVLX8888;
	else if (str::strcasecmp("R32F", arg) == 0)
		return IMAGE_FORMAT_R32F;
	else if (str::strcasecmp("RGB323232F", arg) == 0)
		return IMAGE_FORMAT_RGB323232F;
	else if (str::strcasecmp("RGBA32323232F", arg) == 0)
		return IMAGE_FORMAT_RGBA32323232F;
	else if (str::strcasecmp("NV_NULL", arg) == 0)
		return IMAGE_FORMAT_NV_NULL;
	else if (str::strcasecmp("ATI2N", arg) == 0)
		return IMAGE_FORMAT_ATI2N;
	else if (str::strcasecmp("ATI1N", arg) == 0)
		return IMAGE_FORMAT_ATI1N;
	else if (str::strcasecmp("BC7", arg) == 0)
		return IMAGE_FORMAT_BC7;
	else if (str::strcasecmp("COUNT", arg) == 0)
		return IMAGE_FORMAT_COUNT;
	else
		return IMAGE_FORMAT_NONE;
}
