
#include <iostream>
#include <limits>

#include "action_modify.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

#include "VTFLib.h"

#include "fmt/format.h"

using namespace VTFLib;
using namespace vtex2;

namespace opts
{
	static int recursive;
	static int format;
	static int addflag;
	static int removeflag;
	static int compress;
	static int stripresource;
	static int mips;
	static int file;
} // namespace opts


#undef min // Stupid windows

// Signifies no modification
static constexpr int NO_MODIFY_INT = std::numeric_limits<int>::min();

std::string ActionModify::get_help() const {
	return "Modifies existing VTF files in-place";
}

const OptionList& ActionModify::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::recursive = opts.add(
			ActionOption()
				.short_opt("-r")
				.long_opt("--recursive")
				.type(OptType::Bool)
				.value(false)
				.help("Recursively process directories"));

		opts::format = opts.add(
			ActionOption()
				.short_opt("-f")
				.long_opt("--format")
				.type(OptType::String)
				.value("")
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
				.help("Image format to convert VTFs to"));

		opts::addflag = opts.add(
			ActionOption()
				.short_opt("-af")
				.long_opt("--add-flag")
				.type(OptType::String)
				.value("")
				.choices({
					"pointsample",
					"trilinear",
					"clamps",
					"clampt",
					"anisotropic",
					"hint_dxt5",
					"srgb",
					"nocompress",
					"normal",
					"nomip",
					"nolod",
					"minmip",
					"procedural",
					"onebitalpha",
					"eightbitalpha",
					"envmap",
					"rendertarget",
					"depthrendertarget",
					"nodebugoverride",
					"singlecopy",
					"unused0",
					"oneovermiplevelinalpha",
					"unused1",
					"premultcolorbyoneovermiplevel",
					"unused2",
					"normaltodudv",
					"unused3",
					"alphatestmipgeneration",
					"nodepthbuffer",
					"unused4",
					"nicefiltered",
					"clampu",
					"vertextexture",
					"ssbump",
					"unused5",
					"unfilterable_ok",
					"border",
					"specvar_red",
					"specvar_alpha",
				})
				.help("Flag to add to each VTF file"));

		opts::removeflag = opts.add(
			ActionOption()
				.short_opt("-af")
				.long_opt("--add-flag")
				.type(OptType::String)
				.value("")
				.choices({
					"pointsample",
					"trilinear",
					"clamps",
					"clampt",
					"anisotropic",
					"hint_dxt5",
					"srgb",
					"nocompress",
					"normal",
					"nomip",
					"nolod",
					"minmip",
					"procedural",
					"onebitalpha",
					"eightbitalpha",
					"envmap",
					"rendertarget",
					"depthrendertarget",
					"nodebugoverride",
					"singlecopy",
					"unused0",
					"oneovermiplevelinalpha",
					"unused1",
					"premultcolorbyoneovermiplevel",
					"unused2",
					"normaltodudv",
					"unused3",
					"alphatestmipgeneration",
					"nodepthbuffer",
					"unused4",
					"nicefiltered",
					"clampu",
					"vertextexture",
					"ssbump",
					"unused5",
					"unfilterable_ok",
					"border",
					"specvar_red",
					"specvar_alpha",
				})
				.help("Flag to remove from each VTF file"));

		opts::compress = opts.add(
			ActionOption()
				.short_opt("-c")
				.long_opt("--compress")
				.type(OptType::Int)
				.value(NO_MODIFY_INT)
				.choices({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"})
				.help("DEFLATE compression level to use. 0=none, 9=max. This will force VTF version to 7.6"));

		opts::stripresource = opts.add(
			ActionOption()
				.short_opt("-sr")
				.long_opt("--strip-resource")
				.type(OptType::String)
				.value("")
				.choices({
					"all",
					"low_res_image",
					"image",
					"sheet",
					"crc",
					"texture_lod_settings",
					"texture_settings_ex",
					"key_value_data",
					"aux_compression_info",
				})
				.help("Resource to strip from the VTF file"));

		opts::mips = opts.add(
			ActionOption()
				.short_opt("-m")
				.long_opt("--mips")
				.type(OptType::Int)
				.value(NO_MODIFY_INT)
				.help("Number of mips to generate"));

		opts::file = opts.add(
			ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("VTF file to modify")
				.end_of_line(true)
				.required(true));
	};
	return opts;
}

int ActionModify::exec(const OptionList& opts) {

	const auto recursive = opts.get(opts::recursive).get<bool>();
	const auto file = opts.get(opts::file).get<std::string>();

	if (std::filesystem::is_directory(file)) {
		if (recursive) {
			auto it = std::filesystem::recursive_directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory())
					continue;
				// check that we're actually a vtf file
				auto pathStr = dirent.path().extension().string();
				if (str::strcasecmp(pathStr.c_str(), ".vtf"))
					continue;
				if (!process_file(opts, dirent))
					return 1;
			}
		}
		else {
			auto it = std::filesystem::directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory())
					continue;
				// check that we're actually a vtf file
				auto pathStr = dirent.path().extension().string();
				if (str::strcasecmp(pathStr.c_str(), ".vtf"))
					continue;
				if (!process_file(opts, dirent))
					return 1;
			}
		}
		return 0;
	}
	else {
		return process_file(opts, file) ? 0 : 1;
	}

	return 0;
}

void ActionModify::cleanup() {
}

bool ActionModify::process_file(const OptionList& opts, const std::filesystem::path& srcFile) const {
	auto formatStr = opts.get(opts::format).get<std::string>();
	auto addflag = opts.get(opts::addflag).get<std::string>();
	auto removeflag = opts.get(opts::removeflag).get<std::string>();
	auto compress = opts.get(opts::compress).get<int>();
	auto stripresource = opts.get(opts::stripresource).get<std::string>();
	auto mips = opts.get(opts::mips).get<int>();

	auto* vtfFile = new CVTFFile();

	util::cleanup vtfCleanup(
		[vtfFile]
		{
			delete vtfFile;
		});

	if (!vtfFile->Load(srcFile.string().c_str(), false)) {
		std::cerr << fmt::format("Unable to load {}\n", srcFile.string());
		return false;
	}

	if (!formatStr.empty()) {
		auto format = ImageFormatFromUserString(formatStr.c_str());

		vtfFile->ConvertInPlace(format);
	}

	if (!addflag.empty()) {
		auto flag = TextureFlagFromUserString(addflag.c_str());

		vtfFile->SetFlag(flag, true);
	}

	if (!removeflag.empty()) {
		auto flag = TextureFlagFromUserString(removeflag.c_str());

		vtfFile->SetFlag(flag, false);
	}

	if (compress != NO_MODIFY_INT) {
		if (compress == 0) {
			// Delete compression lump (don't compress)
			vtfFile->SetResourceData(VTF_RSRC_AUX_COMPRESSION_INFO, 0, nullptr);
		} else {
			// Update to latest version
			vtfFile->SetVersion(VTF_MAJOR_VERSION, VTF_MINOR_VERSION);

			vtfFile->SetAuxCompressionLevel(compress);
		}
	}

	if (!stripresource.empty()) {
		auto resource = ResourceFromUserString(stripresource.c_str());

		vtfFile->SetResourceData(resource, 0, nullptr);
	}

	if (mips != NO_MODIFY_INT) {
		// TODO, we could modify the miplevel in the header and regenerate mips but this is a pretty horrible way to do this until we change vtflib
	}

	/*auto tempPath = srcFile;
	tempPath.replace_filename(std::filesystem::path("test.vtf"));
	vtfFile->Save(tempPath.string().c_str());*/

	if (!vtfFile->Save(srcFile.string().c_str())) {
		std::cerr << fmt::format("Unable to save {}\n", srcFile.string());
		return false;
	}

	return true;
}
