/**
 * Convert image to VTF
 *  Lots of TODOs here:
 *   @TODO: Manual mips
 *   @TODO: Multi-frame
 *   @TODO: Multi-face
 *   @TODO: Thumbnail
 */
#include <unordered_map>
#include <functional>
#include <iostream>

#include "fmt/format.h"
#include "VTFLib.h"

#include "action_convert.hpp"
#include "common/enums.hpp"
#include "common/image.hpp"
#include "common/util.hpp"

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

		opts::gammacorrect = opts.add(
			ActionOption().long_opt("--gamma-correct").type(OptType::Float).value(0).help("Apply gamma correction"));

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
	};
	return opts;
}

int ActionConvert::exec(const OptionList& opts) {
	auto outfile = opts.get(opts::output).get<std::string>();
	auto recursive = opts.get(opts::recursive).get<bool>();
	auto file = opts.get(opts::file).get<std::string>();

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
	auto formatStr = opts.get(opts::format).get<std::string>();
	auto mips = opts.get(opts::mips).get<int>();
	auto normal = opts.get(opts::normal).get<bool>();
	auto clamps = opts.get(opts::clamps).get<bool>();
	auto clampt = opts.get(opts::clampt).get<bool>();
	auto clampu = opts.get(opts::clampu).get<bool>();
	auto tri = opts.get(opts::trilinear).get<bool>();
	auto point = opts.get(opts::pointsample).get<bool>();
	auto startFrame = opts.get(opts::startframe).get<int>();
	auto bumpScale = opts.get(opts::bumpscale).get<float>();
	auto gammacorrect = opts.get(opts::gammacorrect).get<float>();
	auto srgb = opts.get(opts::srgb).get<bool>();
	auto thumbnail = opts.get(opts::thumbnail).get<bool>();
	auto verStr = opts.get(opts::version).get<std::string>();
	auto compressionLevel = opts.get(opts::compress).get<int>();

	// If an out file name is not provided, we need to build our own
	std::filesystem::path outFile;
	if (userOutputFile.empty()) {
		outFile = srcFile.parent_path() / srcFile.filename().replace_extension(".vtf");
	} else {
		outFile = srcFile.parent_path()  / userOutputFile;
	}

	auto format = ImageFormatFromUserString(formatStr.c_str());
	auto* vtfFile = new CVTFFile();

	add_image_data(srcFile, vtfFile, format, true);

	util::cleanup vtfCleanup(
		[vtfFile]
		{
			delete vtfFile;
		});

	int majorVer, minorVer;
	if (!get_version_from_str(verStr, majorVer, minorVer)) {
		std::cerr << fmt::format("Invalid version '{}'! Valid versions: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6\n", verStr);
		return false;
	}

	minorVer = (compressionLevel > 0) ? 6 : minorVer; // Force 7.6 if using DEFLATE
	vtfFile->SetVersion(majorVer, minorVer);

	// Set the DEFLATE compression level
	if (compressionLevel > 0 && !vtfFile->SetAuxCompressionLevel(compressionLevel)) {
		std::cerr << fmt::format("Could not set compression level to {}!\n", compressionLevel);
		return false;
	}

	vtfFile->SetFlag(TEXTUREFLAGS_NORMAL, normal);
	vtfFile->SetFlag(TEXTUREFLAGS_CLAMPS, clamps);
	vtfFile->SetFlag(TEXTUREFLAGS_CLAMPT, clampt);
	vtfFile->SetFlag(TEXTUREFLAGS_CLAMPU, clampu);
	vtfFile->SetFlag(TEXTUREFLAGS_TRILINEAR, tri);
	vtfFile->SetFlag(TEXTUREFLAGS_POINTSAMPLE, point);
	vtfFile->SetFlag(TEXTUREFLAGS_SRGB, srgb);

	vtfFile->SetStartFrame(startFrame);
	vtfFile->ComputeReflectivity();
	vtfFile->SetBumpmapScale(bumpScale);

	if (thumbnail && !vtfFile->GenerateThumbnail(srgb)) {
		std::cerr << fmt::format("Could not generate thumbnail: {}\n", vlGetLastError());
		return false;
	}

	if (!vtfFile->GenerateMipmaps(MIPMAP_FILTER_CATROM, srgb)) {
		std::cerr << "Could not generate mipmaps!\n";
		return false;
	}
	if (!vtfFile->Save(outFile.string().c_str())) {
		std::cerr << fmt::format("Could not save file {}: {}\n", outFile.string(), vlGetLastError());
		return false;
	}
	fmt::print("{} -> {}\n", srcFile.string(), outFile.string());

	return true;
}

// Add base image info. Lowest mip level.
bool ActionConvert::add_image_data(
	const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file, VTFImageFormat format, bool create) {
	auto image = imglib::image_begin(imageSrc);
	if (!image)
		return false;

	auto data = imglib::image_load(image);
	util::cleanup dataCleanup(
		[&]()
		{
			imglib::image_free(data);
			imglib::image_end(image);
		});

	if (!data.data)
		return false;

	auto dataFormat = imglib::get_vtf_format(data.info);
	vlByte* dest = nullptr;
	// Convert to requested format, if possible.
	if (format != IMAGE_FORMAT_NONE) {

		auto sizeRequired = CVTFFile::ComputeImageSize(data.info.w, data.info.h, 1, format);
		dest = (vlByte*)malloc(sizeRequired);

		if (!CVTFFile::Convert((vlByte*)data.data, dest, data.info.w, data.info.h, dataFormat, format)) {
			std::cerr << fmt::format(
				"Could not convert from {} to {}!\n", ImageFormatToString(dataFormat), ImageFormatToString(format));
			free(dest);
			return false;
		}
	}
	else {
		format = dataFormat;
	}

	if (create) {
		if (!file->Create(data.info.w, data.info.h, 1, 1, 1, format)) {
			std::cerr << "Could not create VTF.\n";
			free(dest);
			return false;
		}
	}
	file->SetData(1, 1, 1, 0, dest ? dest : (vlByte*)data.data);

	free(dest);
	return true;
}

static bool get_version_from_str(const std::string& str, int& major, int& minor) {
	auto pos = str.find('.');
	if (pos == str.npos)
		return false;
	auto majorVer = str.substr(0, pos);
	auto minorVer = str.substr(pos + 1);
	return util::strtoint(majorVer, major) && util::strtoint(minorVer, minor);
}
