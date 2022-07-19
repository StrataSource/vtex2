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

using namespace VTFLib;
using namespace vtex2;

namespace opts {
	static int output;
	static int format;
	static int file;
	static int recursive;
}

static VTFImageFormat format_for_image(const imglib::ImageInfo_t& info);

std::string ActionConvert::get_help() const {
	return "Convert a generic image file to VTF";
}

const OptionList& ActionConvert::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::output = opts.add( ActionOption()
				.short_opt("-o")
				.long_opt("--output")
				.type(OptType::String)
				.value("")
				.help("Name of the output VTF")
		);
		
		opts::format = opts.add( ActionOption()
				.short_opt("-f")
				.long_opt("--format")
				.type(OptType::String)
				.value("rgba8888")
				.choices({
					"rgba8888", "abgr8888", "rgb888",
					"bgr888", "rgb565", "i8",
					"ia88", "p8", "a8", "rgb888_bluescreen",
					"bgr888_bluescreen", "argb8888", "bgra8888",
					"dxt1", "dxt3", "dxt5", "bgrx8888",
					"bgr565", "bgrx5551", "bgra4444",
					"dxt1_onebitalpha", "bgra5551", "uv88",
					"uvwq8888", "rgba16161616f", "rgba16161616",
					"uvlx8888", "r32f", "rgb323232f",
					"rgba32323232f", "ati2n", "ati1n",
				})
				.help("Image format of the VTF")
		);
		
		opts::file = opts.add( ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("Image file to convert")
				.required(true)
				.end_of_line(true)
		);
		
		opts::recursive = opts.add( ActionOption()
				.short_opt("-r")
				.long_opt("--recursive")
				.type(OptType::Bool)
				.value(false)
				.help("Recursively process directories")
		);
	};
	return opts;
}

int ActionConvert::exec(const OptionList& opts) {
	auto outfile = opts.get(opts::output).get<std::string>();
	auto recursive = opts.get(opts::recursive).get<bool>();
	auto file = opts.get(opts::file).get<std::string>();
	
	if (std::filesystem::is_directory(file)) {
		// Recursively process dirs
		std::function<bool(const std::filesystem::path&)> processDir = 
			[&opts, &processDir, this, recursive](const std::filesystem::path& path) {
			auto it = std::filesystem::directory_iterator(path);
			for (auto& dirent : it) {
				if (dirent.is_directory() && !processDir(dirent))
					return false;
				// check that we're actually a convertable file
				if (imglib::image_get_format_from_file(path.string().c_str()) == imglib::FileFormat::None)
					continue;
				if (!process_file(opts, dirent, ""))
					return false;
			}
			return true;
		};
		return processDir(file) ? 0 : 1;
	}
	else {
		return process_file(opts, file, outfile) ? 0 : 1;
	}
}

void ActionConvert::cleanup() {
	
}

bool ActionConvert::process_file(const OptionList& opts, const std::filesystem::path& srcFile, const std::filesystem::path& userOutputFile) {
	auto formatStr = opts.get(opts::format).get<std::string>();
	
	// If an out file name is not provided, we need to build our own
	std::filesystem::path outFile;
	if (userOutputFile.empty()) {
		outFile = srcFile.parent_path() / srcFile.filename().replace_extension(".vtf");
	}
	
	auto format = ImageFromatFromUserString(formatStr.c_str());
	if (format == IMAGE_FORMAT_NONE) {
		std::cerr << fmt::format("Invalid pixel format: '{}'\n", formatStr);
		return false;
	}
	
	auto* vtfFile = new CVTFFile();
	add_image_data(srcFile, vtfFile);
	
	if (!vtfFile->Save(outFile.string().c_str())) {
		std::cerr << fmt::format("Could not save file {}\n", outFile.string());
		return false;
	}
	fmt::print("{} -> {}\n", srcFile.string(), outFile.string());
	
	delete vtfFile;
	return true;
}

// Add base image info. Lowest mip level.
bool ActionConvert::add_image_data(const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file) {
	
	auto image = imglib::image_begin(imageSrc);
	if (!image)
		return false;
		
	auto data = imglib::image_load(image);
	
	file->Create(data.info.w, data.info.h, 1, 1, 1, format_for_image(data.info));
	file->SetData(1, 1, 1, 0, (vlByte*)data.data);
	
	imglib::image_end(image);
	imglib::image_free(data);
	
	return true;
}

// Determine the VTF format we want to use for the image
static VTFImageFormat format_for_image(const imglib::ImageInfo_t& info) {
	if (info.comps == 3) {
		if (info.type == imglib::Float)
			return IMAGE_FORMAT_RGB323232F;
		return info.type == imglib::UInt8 ? IMAGE_FORMAT_RGB888 : IMAGE_FORMAT_RGBA16161616;
	}
	else {
		if (info.type == imglib::Float)
			return IMAGE_FORMAT_RGBA32323232F;
		return info.type == imglib::UInt8 ? IMAGE_FORMAT_RGBA8888 : IMAGE_FORMAT_RGBA16161616;
	}
}
