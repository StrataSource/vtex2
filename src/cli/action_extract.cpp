
#include <filesystem>
#include <iostream>
#include <functional>

#include "nameof.hpp"
#include "fmt/format.h"

#include "action_extract.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"
#include "common/strtools.hpp"
#include "common/image.hpp"

#include "VTFLib.h"

using namespace vtex2;

namespace opts
{
	static int output;
	static int file;
	static int format;
	static int mip;
	static int recursive;
	static int noalpha;
} // namespace opts

std::string ActionExtract::get_help() const {
	return "Converts a VTF into png, tga, jpeg, bmp or hdr image file";
}

const OptionList& ActionExtract::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::output = opts.add(
			ActionOption()
				.short_opt("-o")
				.long_opt("--output")
				.type(OptType::String)
				.value("")
				.help("File to place the output in"));

		opts::format = opts.add(
			ActionOption()
				.short_opt("-f")
				.long_opt("--format")
				.type(OptType::String)
				.value("")
				.choices({"png", "jpeg", "jpg", "tga", "bmp", "hdr"})
				.help("Output format to use"));

		opts::file = opts.add(
			ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("VTF file to convert or directory to process")
				.required(true)
				.end_of_line(true));

		opts::mip = opts.add(
			ActionOption()
				.short_opt("-m")
				.long_opt("--mip")
				.type(OptType::Int)
				.value(0)
				.help("Mipmap to extract from image"));

		opts::recursive = opts.add(
			ActionOption()
				.short_opt("-r")
				.long_opt("--recursive")
				.type(OptType::Bool)
				.value(false)
				.help("Recursively process directories"));

		opts::noalpha = opts.add(
			ActionOption()
				.short_opt("-na")
				.long_opt("--no-alpha")
				.type(OptType::Bool)
				.value(false)
				.help("Exclude alpha channel from converted image"));
	};
	return opts;
}

int ActionExtract::exec(const OptionList& opts) {

	std::filesystem::path file = opts.get<std::string>(opts::file);
	std::filesystem::path output = opts.get<std::string>(opts::output);
	const bool recursive = opts.get<bool>(opts::recursive);

	if (std::filesystem::is_directory(file)) {
		if (recursive) {
			auto it = std::filesystem::recursive_directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory() || dirent.path().extension() != ".vtf")
					continue;
				if (!extract_file(opts, dirent, ""))
					return 1;
			}
		}
		else {
			auto it = std::filesystem::directory_iterator(file);
			for (auto& dirent : it) {
				if (dirent.is_directory() || dirent.path().extension() != ".vtf")
					continue;
				if (!extract_file(opts, dirent, ""))
					return 1;
			}
		}
		return 0;
	}
	else {
		return extract_file(opts, file, output) ? 0 : 1;
	}

	return 0;
}

void ActionExtract::cleanup() {
	delete file_;
}

bool ActionExtract::extract_file(
	const OptionList& opts, const std::filesystem::path& vtfPath, const std::filesystem::path& userOutputFile) {
	if (!load_vtf(vtfPath))
		return false;

	auto format = opts.get<std::string>(opts::format);
	auto mip = opts.get<int>(opts::mip);
	const bool noalpha = opts.get<bool>(opts::noalpha);

	// If the user provided output file is empty, we'll determine a default
	auto outFile = userOutputFile;
	if (userOutputFile.empty()) {
		if (format.empty()) {
			std::cerr << "Missing --format parameter for batch processing\n";
			return false;
		}

		auto eFmt = imglib::image_get_format(format.c_str());
		if (eFmt == imglib::FileFormat::None) {
			std::cerr << "Could not determine file format from --format parameter\n";
			return false;
		}

		// Now build a default file name
		auto* ext = imglib::image_get_extension(imglib::image_get_format(format.c_str()));
		outFile = vtfPath.parent_path() / vtfPath.filename().replace_extension(ext);
	}

	fmt::print("{} -> {}\n", vtfPath.string(), outFile.string());

	// Validate mipmap selection
	if (mip > file_->GetMipmapCount()) {
		std::cerr << fmt::format(
			"Selected mip {} exceeds the total mip count of the image: {}\n", mip, file_->GetMipmapCount());
		return false;
	}

	// Determine format based on output file extension
	std::string targetFormatName = str::get_ext(outFile.string().c_str());
	if (!format.empty())
		targetFormatName = format;
	auto targetFmt = imglib::image_get_format(targetFormatName.c_str());

	// Ensure target file format is valid
	if (targetFmt == imglib::FileFormat::None) {
		std::cerr << fmt::format(
			"Could not determine file format from file '{}'. To explicitly choose a format, pass --format.\n",
			outFile.string());
		return false;
	}

	vlUInt w, h, d;
	file_->ComputeMipmapDimensions(file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip, w, h, d);

	auto formatInfo = file_->GetImageFormatInfo(file_->GetFormat());
	int comps = formatInfo.uiAlphaBitsPerPixel > 0 ? 4 : 3;

	// Do we want to exclude alpha channel??
	if (noalpha)
		comps = 3;

	// Allocate buffer for image data
	vlByte* imageData = nullptr;
	auto imageDataClean = util::cleanup(
		[imageData]
		{
			free(imageData);
		});

	// Only supported format for Hdr is 32-bit RGBA - everything else will be squashed down into 32 or 24bpp RGB/RGBA
	bool destIsFloat = (targetFmt == imglib::Hdr);
	bool ok = false;
	if (destIsFloat) {
		imageData = static_cast<vlByte*>(malloc(w * h * comps * sizeof(float)));
		ok = VTFLib::CVTFFile::Convert(
			file_->GetData(0, 0, 0, mip), imageData, w, h, file_->GetFormat(),
			comps == 3 ? IMAGE_FORMAT_RGB323232F : IMAGE_FORMAT_RGBA32323232F);
	}
	else {
		// comps = 3;
		imageData = static_cast<vlByte*>(malloc(w * h * comps * sizeof(uint8_t)));
		ok = VTFLib::CVTFFile::Convert(
			file_->GetData(0, 0, 0, mip), imageData, w, h, file_->GetFormat(),
			comps == 3 ? IMAGE_FORMAT_RGB888 : IMAGE_FORMAT_RGBA8888);
	}

	if (!ok) {
		std::cerr << fmt::format(
			"Could not convert image format '{}' -> '{}'!\n", NAMEOF_ENUM(file_->GetFormat()),
			destIsFloat ? "RGBA32323232F" : "RGBA8888");
		return false;
	}

	imglib::Image image(imageData, destIsFloat ? imglib::Float : imglib::UInt8, comps, w, h, true);
	if (!image.save(outFile.string().c_str(), targetFmt)) {
		std::cerr << fmt::format("Could not save image to '{}'!\n", outFile.string());
		return false;
	}

	return true;
}

bool ActionExtract::load_vtf(const std::filesystem::path& vtfFile) {
	// Cleanup any existing files
	if (file_)
		delete file_;

	// Load off disk
	std::uint8_t* buf = nullptr;
	auto numBytes = util::read_file(vtfFile.string(), buf);
	auto bufCleanup = util::cleanup(
		[&buf]
		{
			delete[] buf;
		});

	if (numBytes == 0 || !buf) {
		std::cerr << fmt::format("Could not open file '{}'!\n", vtfFile.string());
		delete[] buf;
		return false;
	}

	// Create new file & load it with vtflib
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(buf, numBytes, false)) {
		std::cerr << fmt::format("Failed to load VTF '{}': {}\n", vtfFile.string(), vlGetLastError());
		return false;
	}

	return true;
}
