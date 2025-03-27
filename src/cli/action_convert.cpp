/**
 * Implements both convert and modify actions
 *  Lots of TODOs here:
 *   @TODO: Manual mips
 *   @TODO: Multi-frame
 *   @TODO: Multi-face
 *   @TODO: Thumbnail
 */
#include <unordered_map>
#include <functional>
#include <iostream>
#include <algorithm>

#include "nameof.hpp"
#include "fmt/format.h"
#include "VTFLib.h"

#include "action_convert.hpp"
#include "common/enums.hpp"
#include "common/image.hpp"
#include "common/util.hpp"
#include "common/vtftools.hpp"

// Windows garbage!!
#undef min
#undef max

using namespace VTFLib;
using namespace vtex2;

namespace opts
{
	static int output;
	static int format;
	static int file;
	static int recursive;
	static int mips;
	static int normal;
	static int clamps, clampt, clampu;
	static int pointsample, trilinear;
	static int startframe, bumpscale;
	static int gammacorrect, srgb;
	static int thumbnail;
	static int version;
	static int compress;
	static int width, height;
	static int nomips;
	static int toDX;
	static int quiet;
	static int swizzle;
} // namespace opts

static bool get_version_from_str(const std::string& str, int& major, int& minor);

std::string ActionConvert::get_help() const {
	return "Convert a generic image file to VTF";
}

const OptionList& ActionConvert::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::output = opts.add(
			ActionOption()
				.short_opt("-o")
				.long_opt("--output")
				.type(OptType::String)
				.value("")
				.help("Name of the output VTF"));

		opts::format = opts.add(
			ActionOption()
				.short_opt("-f")
				.long_opt("--format")
				.type(OptType::String)
				.value("rgba8888")
				.choices({
					"rgba8888",
					"abgr8888",
					"rgb888",
					"bgr888",
					"rgb565",
					"i8",
					"ia88",
					"p8",
					"a8",
					"rgb888_bluescreen",
					"bgr888_bluescreen",
					"argb8888",
					"bgra8888",
					"dxt1",
					"dxt3",
					"dxt5",
					"bgrx8888",
					"bgr565",
					"bgrx5551",
					"bgra4444",
					"dxt1_onebitalpha",
					"bgra5551",
					"uv88",
					"uvwq8888",
					"rgba16161616f",
					"rgba16161616",
					"uvlx8888",
					"r32f",
					"rgb323232f",
					"rgba32323232f",
					"ati2n",
					"ati1n",
					"bc7",
				})
				.help("Image format of the VTF"));

		opts::file = opts.add(
			ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("Image file to convert or directory to process")
				.required(true)
				.end_of_line(true));

		opts::recursive = opts.add(
			ActionOption()
				.short_opt("-r")
				.long_opt("--recursive")
				.type(OptType::Bool)
				.value(false)
				.help("Recursively process directories"));

		opts::thumbnail = opts.add(
			ActionOption()
				.long_opt("--thumbnail")
				.type(OptType::Bool)
				.value(false)
				.help("Generate thumbnail for the image"));

		opts::normal = opts.add(
			ActionOption()
				.short_opt("-n")
				.long_opt("--normal")
				.type(OptType::Bool)
				.value(false)
				.help("Create a normal map"));

		opts::clamps =
			opts.add(ActionOption().long_opt("--clamps").type(OptType::Bool).value(false).help("Clamp on S axis"));

		opts::clampt =
			opts.add(ActionOption().long_opt("--clampt").type(OptType::Bool).value(false).help("Clamp on T axis"));

		opts::clampu =
			opts.add(ActionOption().long_opt("--clampu").type(OptType::Bool).value(false).help("Clamp on U axis"));

		opts::pointsample = opts.add(
			ActionOption()
				.long_opt("--pointsample")
				.type(OptType::Bool)
				.value(false)
				.help("Set point sampling method"));

		opts::trilinear = opts.add(
			ActionOption()
				.long_opt("--trilinear")
				.type(OptType::Bool)
				.value(false)
				.help("Set trilinear sampling method"));

		opts::mips = opts.add(
			ActionOption()
				.short_opt("-m")
				.long_opt("--mips")
				.type(OptType::Int)
				.value(10)
				.help("Number of mips to generate"));

		opts::startframe = opts.add(
			ActionOption().long_opt("--start-frame").type(OptType::Int).value(0).help("Animation frame to start on"));

		opts::bumpscale =
			opts.add(ActionOption().long_opt("--bumpscale").type(OptType::Float).value(0).help("Bumpscale"));

		opts::srgb = opts.add(
			ActionOption()
				.long_opt("--srgb")
				.type(OptType::Bool)
				.value(false)
				.help("Process this image in sRGB color space"));

		opts::version = opts.add(
			ActionOption().long_opt("--version").type(OptType::String).value("7.5").help("Set the VTF version to use"));

		opts::compress = opts.add(
			ActionOption()
				.short_opt("-c")
				.long_opt("--compress")
				.type(OptType::Int)
				.value(0)
				.choices({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"})
				.help("DEFLATE compression level to use. 0=none, 9=max. This will force VTF version to 7.6"));

		opts::width = opts.add(
			ActionOption()
				.short_opt("-w")
				.long_opt("--width")
				.type(OptType::Int)
				.value(-1)
				.help("Width of the output VTF"));

		opts::height = opts.add(
			ActionOption()
				.short_opt("-h")
				.long_opt("--height")
				.type(OptType::Int)
				.value(-1)
				.help("Height of the output VTF"));

		opts::nomips = opts.add(
			ActionOption()
				.long_opt("--no-mips")
				.type(OptType::Bool)
				.value(false)
				.help("Disable mipmaps for this texture"));

		opts::toDX = opts.add(
			ActionOption()
				.long_opt("--opengl")
				.short_opt("-gl")
				.value(false)
				.type(OptType::Bool)
				.help("Treat the incoming normal map as an OpenGL normal map"));

		opts::quiet = opts.add(
			ActionOption()
				.long_opt("--quiet")
				.short_opt("-q")
				.value(false)
				.type(OptType::Bool)
				.help("Silence output messages that aren't errors")
		);

		opts::swizzle = opts.add(
			ActionOption()
				.long_opt("--swizzle")
				.type(OptType::String)
				.help("Perform an in-place swizzle on the image data")
		);
	};
	return opts;
}

int ActionConvert::exec(const OptionList& opts) {
	auto outfile = opts.get<std::string>(opts::output);
	auto recursive = opts.get<bool>(opts::recursive);
	auto file = opts.get<std::string>(opts::file);

	if (std::filesystem::is_directory(file)) {
		if (recursive) {
			auto it = std::filesystem::recursive_directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory())
					continue;
				// check that we're actually a convertable file
				if (imglib::image_get_format_from_file(dirent.path().string().c_str()) == imglib::FileFormat::None)
					continue;
				if (!process_file(opts, dirent, ""))
					return 1;
			}
		}
		else {
			auto it = std::filesystem::directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory())
					continue;
				// check that we're actually a convertable file
				if (imglib::image_get_format_from_file(dirent.path().string().c_str()) == imglib::FileFormat::None)
					continue;
				if (!process_file(opts, dirent, ""))
					return 1;
			}
		}
		return 0;
	}
	else {
		return process_file(opts, file, outfile) ? 0 : 1;
	}
}

void ActionConvert::cleanup() {
}

bool ActionConvert::process_file(
	const OptionList& opts, const std::filesystem::path& srcFile, const std::filesystem::path& userOutputFile) {

	m_opts = &opts;

	const auto formatStr = opts.get<std::string>(opts::format);
	const auto srgb = opts.get<bool>(opts::srgb);
	const auto thumbnail = opts.get<bool>(opts::thumbnail);
	const auto verStr = opts.get<std::string>(opts::version);
	const auto isNormal = opts.get<bool>(opts::normal);

	auto nomips = opts.get<bool>(opts::nomips);
	m_mips = nomips ? 1 : std::max(opts.get<int>(opts::mips), 1);
	// If mips is not provided, we'll use a default later on
	if (!opts.has(opts::mips) && !nomips)
		m_mips = -1;

	m_width = opts.get<int>(opts::width);
	m_height = opts.get<int>(opts::height);

	if (!std::filesystem::exists(srcFile)) {
		std::cerr << "Could not open " << srcFile << ": file does not exist\n";
		return false;
	}

	bool isvtf = srcFile.filename().extension() == ".vtf";

	// If an out file name is not provided, we need to build our own
	std::filesystem::path outFile;
	if (userOutputFile.empty()) {
		outFile = srcFile.parent_path() / srcFile.filename().replace_extension(".vtf");
	}
	else {
		outFile = userOutputFile;
	}

	auto format = ImageFormatFromUserString(formatStr.c_str());
	auto vtfFile = std::make_unique<CVTFFile>();

	// We will choose the best format to operate on here. This simplifies later code and lets us avoid extraneous
	// conversions
	auto formatInfo = CVTFFile::GetImageFormatInfo(format);
	imglib::ChannelType procChanType;
	const auto procFormat = [formatInfo, &procChanType]() -> VTFImageFormat
	{
		auto maxBpp = std::max(
			std::max(formatInfo.uiRedBitsPerPixel, formatInfo.uiGreenBitsPerPixel),
			std::max(formatInfo.uiBlueBitsPerPixel, formatInfo.uiAlphaBitsPerPixel));
		if (maxBpp > 16) {
			procChanType = imglib::ChannelType::Float;
			return IMAGE_FORMAT_RGBA32323232F;
		}
		else if (maxBpp > 8) {
			procChanType = imglib::ChannelType::UInt16;
			return IMAGE_FORMAT_RGBA16161616;
		}
		else {
			procChanType = imglib::ChannelType::UInt8;
			return IMAGE_FORMAT_RGBA8888;
		}
	}();

	// If we're processing a VTF, let's add that VTF image data
	size_t initialSize = 0;
	if (isvtf) {
		auto* srcVtf = init_from_file(srcFile, vtfFile.get(), procFormat);
		if (!srcVtf) {
			std::cerr << fmt::format("Could not open {}\n", srcFile.string());
			return false;
		}

		initialSize = srcVtf->GetSize();

		if (!add_vtf_image_data(srcVtf, vtfFile.get(), procFormat)) {
			delete srcVtf;
			std::cerr << "Could not add image data\n";
			return false;
		}
		delete srcVtf;
	}
	// Add standard image data
	else if (!add_image_data(srcFile, vtfFile.get(), procFormat, true)) {
		std::cerr << "Could not add image data from file " << srcFile << "\n";
		return false;
	}

	// Process the image if necessary
	if (isNormal && opts.get<bool>(opts::toDX)) {
		auto image = std::make_shared<imglib::Image>(
			vtfFile->GetData(0, 0, 0, 0), procChanType, 4, vtfFile->GetWidth(), vtfFile->GetHeight(), true);
		if (!image->process(imglib::PROC_GL_TO_DX_NORM)) {
			std::cerr << "Could not process vtf\n";
			return false;
		}
	}

	// Swizzle the image if requested
	if (opts.has(opts::swizzle)) {
		uint32_t mask = imglib::swizzle_from_str(opts.get<std::string>(opts::swizzle).data());
		if (mask != lwiconv::NO_SWIZZLE) {
			auto image = std::make_shared<imglib::Image>(
				vtfFile->GetData(0, 0, 0, 0), procChanType, 4, vtfFile->GetWidth(), vtfFile->GetHeight(), true);
			if (!image->swizzle(mask)) {
				std::cerr << "Could not swizzle vtf\n";
				return false;
			}
		}
	}

	// Set the properties based on user input
	if (!set_properties(vtfFile.get())) {
		std::cerr << "Could not set properties on VTF\n";
		return false;
	}

	// Generate thumbnail
	if (thumbnail && !vtfFile->GenerateThumbnail(srgb)) {
		std::cerr << fmt::format("Could not generate thumbnail: {}\n", util::get_last_vtflib_error());
		return false;
	}

	// Generate mips
	if (!vtfFile->GenerateMipmaps(MIPMAP_FILTER_CATROM, srgb)) {
		std::cerr << "Could not generate mipmaps!\n";
		return false;
	}

	// Convert to desired image format
	if (vtfFile->GetFormat() != format && !vtfFile->ConvertInPlace(format)) {
		std::cerr << fmt::format("Could not convert image data to {}: {}\n", formatStr, util::get_last_vtflib_error());
		return false;
	}

	// Save to disk finally
	if (!vtfFile->Save(outFile.string().c_str())) {
		std::cerr << fmt::format("Could not save file {}: {}\n", outFile.string(), util::get_last_vtflib_error());
		return false;
	}

	// Report file sizes
	if (!opts.get<bool>(opts::quiet)) {
		if (initialSize != 0) {
			fmt::print(
				"{} ({} KiB) -> {} ({} KiB)\n", srcFile.string(), initialSize / 1024, outFile.string(), vtfFile->GetSize() / 1024);
		}
		else {
			fmt::print("{} -> {} ({} KiB)\n", srcFile.string(), outFile.string(), vtfFile->GetSize() / 1024);
		}
	}

	return true;
}

//
// Loads a VTF from file, copies over flags and other properties to `file`
// and then returns the newly loaded file
// If load failed, we'll return nullptr
//
VTFLib::CVTFFile*
ActionConvert::init_from_file(const std::filesystem::path& src, VTFLib::CVTFFile* file, VTFImageFormat newFormat) {
	auto srcFile = new CVTFFile();
	if (!srcFile->Load(src.string().c_str(), false))
		return nullptr;

	// Convert immediately to the processing format, so we can match between src and dest
	srcFile->ConvertInPlace(newFormat);

	// Determine buffer sizes
	const auto width = (m_width == -1) ? srcFile->GetWidth() : m_width;
	const auto height = (m_height == -1) ? srcFile->GetHeight() : m_height;

	// If the width/height have been changed, we can't just pull the mip count from the source VTF
	const bool bNeedsNewMips = width != srcFile->GetWidth() || height != srcFile->GetHeight();
	const int nDefaultMips = CVTFFile::ComputeMipmapCount(width, height, 1);

	// Use the mip count from the source file if the user hasn't specified it yet
	auto mipCount = m_opts->has(opts::mips) || m_opts->has(opts::nomips) 
		? m_mips 
		: (bNeedsNewMips ? nDefaultMips : srcFile->GetMipmapCount());

	// Init image with the desired parameters and processing format
	file->Init(
		width, height, srcFile->GetFrameCount(), srcFile->GetFaceCount(), srcFile->GetDepth(), newFormat,
		srcFile->GetHasThumbnail(), mipCount);

	file->SetFlags(srcFile->GetFlags());
	file->SetVersion(srcFile->GetMajorVersion(), srcFile->GetMinorVersion());

	if (srcFile->GetHasThumbnail())
		file->SetThumbnailData(srcFile->GetThumbnailData());

	return srcFile;
}

//
// Set properties for a VTF based on the user's input
//
bool ActionConvert::set_properties(VTFLib::CVTFFile* vtfFile) {
	auto compressionLevel = m_opts->get<int>(opts::compress);

	// Set version if provided, or if we need it specifically to be 7.6
	if (m_opts->has(opts::version) || compressionLevel > 0 || vtfFile->GetFormat() == IMAGE_FORMAT_BC7) {
		auto verStr = m_opts->get<std::string>(opts::version);

		int majorVer, minorVer;
		if (!get_version_from_str(verStr, majorVer, minorVer)) {
			std::cerr << fmt::format("Invalid version '{}'! Valid versions: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6\n", verStr);
			return false;
		}

		minorVer = (compressionLevel > 0) ? 6 : minorVer; // Force 7.6 if using DEFLATE
		vtfFile->SetVersion(majorVer, minorVer);
	}

	// Set the DEFLATE compression level
	if (!vtfFile->SetAuxCompressionLevel(compressionLevel) && compressionLevel != 0) {
		std::cerr << fmt::format("Could not set compression level to {}!\n", compressionLevel);
		return false;
	}

	// These should be defaulted to off
	// we're not going to set them explicitly to the value of the opts because we may have gotten them from another vtf
	if (m_opts->get<bool>(opts::normal))
		vtfFile->SetFlag(TEXTUREFLAGS_NORMAL, true);
	if (m_opts->get<bool>(opts::clamps))
		vtfFile->SetFlag(TEXTUREFLAGS_CLAMPS, true);
	if (m_opts->get<bool>(opts::clamps))
		vtfFile->SetFlag(TEXTUREFLAGS_CLAMPT, true);
	if (m_opts->get<bool>(opts::clampt))
		vtfFile->SetFlag(TEXTUREFLAGS_CLAMPU, true);
	if (m_opts->get<bool>(opts::trilinear))
		vtfFile->SetFlag(TEXTUREFLAGS_TRILINEAR, true);
	if (m_opts->get<bool>(opts::pointsample))
		vtfFile->SetFlag(TEXTUREFLAGS_POINTSAMPLE, true);
	if (m_opts->get<bool>(opts::srgb))
		vtfFile->SetFlag(TEXTUREFLAGS_SRGB, true);

	// Mip count gets set earlier by user input
	if (vtfFile->GetMipmapCount() == 1)
		vtfFile->SetFlag(TEXTUREFLAGS_NOMIP, true);

	// Same deal for the below issues- only override default if specified
	if (m_opts->has(opts::startframe))
		vtfFile->SetStartFrame(m_opts->get<int>(opts::startframe));

	vtfFile->ComputeReflectivity();

	if (m_opts->has(opts::bumpscale))
		vtfFile->SetBumpmapScale(m_opts->get<float>(opts::bumpscale));

	return true;
}

//
// Add base image data to the VTF's lowest mip level
// imageSrc is a path to a imglib-compatible image
//
bool ActionConvert::add_image_data(
	const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file, VTFImageFormat format, bool create) {

	// Load the image
	auto image = imglib::Image::load(imageSrc);
	if (!image)
		return false;

	// If width and height are specified, resize in place
	if (m_height != -1 && m_width != -1) {
		if (!image->resize(m_width, m_height))
			return false;
	}

	// Hack for VTFLib; Ensure we have an alpha channel because that's well supported in that horrible code
	if (image->channels() < 4 && image->type() != imglib::ChannelType::UInt8) {
		if (!image->convert(image->type(), 4)) {
			std::cerr << fmt::format("Failed to convert {}\n", imageSrc.string());
			return false;
		}
	}

	// Add the raw image data
	return add_image_data_raw(
		file, image->data(), format, image->vtf_format(), image->width(), image->height(), create);
}

//
// Add image data from an existing VTF to the image
//  imageSrc: Path to the existing VTF - This may be modified if resizing or if conversion is needed!!!
//  file: Destination VTF
//  format: Dest format of the data, file->GetFormat() will return this when this returns true
//
bool ActionConvert::add_vtf_image_data(CVTFFile* srcFile, VTFLib::CVTFFile* file, VTFImageFormat format) {
	const auto frameCount = srcFile->GetFrameCount();
	const auto faceCount = srcFile->GetFaceCount();
	const auto sliceCount = srcFile->GetDepth();
	const auto srcWidth = srcFile->GetWidth();
	const auto srcHeight = srcFile->GetHeight();

	assert(file->GetFormat() == format);
	assert(srcFile->GetFormat() == format);

	// Resize VTF only if necessary (This is expensive and kinda crap)
	if (m_width != -1 && m_height != -1 && (srcWidth != m_width || srcHeight != m_height)) {
		return vtf::resize(srcFile, m_width, m_height, file);
	}
	else {
		// Load all image data normally
		for (vlUInt uiFrame = 0; uiFrame < frameCount; ++uiFrame) {
			for (vlUInt uiFace = 0; uiFace < faceCount; ++uiFace) {
				for (vlUInt uiSlice = 0; uiSlice < sliceCount; ++uiSlice) {
					// Load the data normally for once
					file->SetData(uiFrame, uiFace, uiSlice, 0, srcFile->GetData(uiFrame, uiFace, uiSlice, 0));
				}
			}
		}
		return true;
	}
}

//
// Add raw image data to the VTF
//  file is the vtf to add to
//  format is the desired format of the VTF- If set to NONE, we'll use the data format
//
bool ActionConvert::add_image_data_raw(
	VTFLib::CVTFFile* file, const void* data, VTFImageFormat format, VTFImageFormat dataFormat, int w, int h,
	bool create) {
	vlByte* dest = nullptr;

	// Convert to requested format, if necessary
	if (format != IMAGE_FORMAT_NONE && format != dataFormat) {

		auto sizeRequired = CVTFFile::ComputeImageSize(w, h, 1, 1, format);
		dest = (vlByte*)malloc(sizeRequired);

		if (!CVTFFile::Convert((vlByte*)data, dest, w, h, dataFormat, format)) {
			std::cerr << fmt::format(
				"Could not convert from {} to {}: {}\n", NAMEOF_ENUM(dataFormat), NAMEOF_ENUM(format),
				util::get_last_vtflib_error());
			free(dest);
			return false;
		}
	}
	else {
		format = dataFormat;
	}

	// Create the file if we're told to do so
	// This is done here because we don't actually know w/h until now
	if (create) {
		if (!file->Init(w, h, 1, 1, 1, format, vlTrue, m_mips <= 0 ? CVTFFile::ComputeMipmapCount(w, h, 1) : m_mips)) {
			std::cerr << "Could not create VTF: " << util::get_last_vtflib_error() << "\n";
			free(dest);
			return false;
		}
	}
	file->SetData(1, 1, 1, 0, dest ? dest : (vlByte*)data);

	return true;
}

// Get VTF version from string ie 7.6
static bool get_version_from_str(const std::string& str, int& major, int& minor) {
	auto pos = str.find('.');
	if (pos == str.npos)
		return false;
	auto majorVer = str.substr(0, pos);
	auto minorVer = str.substr(pos + 1);
	return util::strtoint(majorVer, major) && util::strtoint(minorVer, minor);
}
