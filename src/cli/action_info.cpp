
#include <iostream>

#include "action_info.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

#include "VTFLib.h"

#include "nameof.hpp"
#include "fmt/format.h"

using namespace vtex2;

namespace opts
{
	static int all;
	static int file;
	static int resources;
} // namespace opts

std::string ActionInfo::get_help() const {
	return "Displays info about a VTF file";
}

const OptionList& ActionInfo::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::all = opts.add(
			ActionOption()
				.long_opt("--all")
				.short_opt("-a")
				.type(OptType::Bool)
				.value(false)
				.help("Display all detailed info about a VTF"));

		opts::resources = opts.add(
			ActionOption()
				.long_opt("--resources")
				.short_opt("-r")
				.type(OptType::Bool)
				.value(false)
				.help("List all resource entries in the VTF"));

		opts::file = opts.add(
			ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("VTF file to process")
				.end_of_line(true)
				.required(true));
	};
	return opts;
}

int ActionInfo::exec(const OptionList& opts) {

	const auto file = opts.get<std::string>(opts::file);
	const auto details = opts.get<bool>(opts::all);
	const auto resources = opts.get<bool>(opts::resources) || details;

	// Load off disk
	std::uint8_t* buf = nullptr;
	auto numBytes = util::read_file(file, buf);
	auto bufCleanup = util::cleanup(
		[&]
		{
			delete[] buf;
		});

	if (numBytes == 0 || !buf) {
		std::cerr << fmt::format(FMT_STRING("Could not open file '{}'!\n"), file);
		return 1;
	}

	// Load VTF with vtflib
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(buf, numBytes, false)) {
		std::cerr << fmt::format(FMT_STRING("Failed to load VTF '{}': {}\n"), file, vlGetLastError());
		return 1;
	}

	auto fmt = file_->GetFormat();

	// Basic compact info mode
	if (!details) {
		compact_info();
		return 0;
	}

	fmt::print(FMT_STRING("VTF Version {}.{}\n"), file_->GetMajorVersion(), file_->GetMinorVersion());
	fmt::print(FMT_STRING("Image format: {}\n"), NAMEOF_ENUM(fmt));
	fmt::print(
		FMT_STRING("Dimensions (WxHxD): {} x {} x {}\n"), file_->GetWidth(), file_->GetHeight(), file_->GetDepth());
	fmt::print(
		FMT_STRING("{} frame(s), {} face(s), {} mipmaps\n"), file_->GetFrameCount(), file_->GetFaceCount(),
		file_->GetMipmapCount());

	if (file_->GetMajorVersion() >= 7 && file_->GetMinorVersion() >= 6) {
		fmt::print(FMT_STRING("DEFLATE compression level {}\n"), file_->GetAuxCompressionLevel());
	}

	if (details) {
		vlUInt dataSize;
		if (auto* crcPtr = file_->GetResourceData(VTF_RSRC_CRC, dataSize)) {
			auto crc = *static_cast<std::uint32_t*>(crcPtr);
			fmt::print(FMT_STRING("Source CRC: 0x{:X}\n"), crc);
		}
		else
			fmt::print("Source CRC: None\n");

		// Display list of texture flags
		auto flags = TextureFlagsToStringVector(file_->GetFlags());
		fmt::print(FMT_STRING("Flags: 0x{:X}\n"), file_->GetFlags());
		for (auto& fl : flags) {
			fmt::print(FMT_STRING("    {}\n"), fl);
		}

		fmt::print(FMT_STRING("Bumpscale: {}\n"), file_->GetBumpmapScale());
		vlSingle x, y, z;
		file_->GetReflectivity(x, y, z);
		fmt::print(FMT_STRING("Reflectivity: ({} {} {})\n"), x, y, z);
	}

	if (resources) {
		const auto rsrcCount = file_->GetResourceCount();
		rsrcCount > 0 ? fmt::print("Resource entries:\n") : fmt::print("No resource entries\n");

		for (auto i = 0; i < rsrcCount; ++i) {
			auto type = file_->GetResourceType(i);
			vlUInt sz = 0;
			file_->GetResourceData(type, sz);
			fmt::print(
				FMT_STRING("    0x{:X} ({:c}{:c}{:c}) - {} bytes ({:1f} KiB)\n"), type, type & 0xFF, (type >> 8) & 0xFF,
				(type >> 16) & 0xFF, sz, sz / 1024.f);
		}
	}

	fmt::print(
		FMT_STRING("{:2f} KiB image data ({:2f} MiB)\n"), file_->GetSize() / 1024.f,
		file_->GetSize() / (1024.f * 1024.f));

	return 0;
}

void ActionInfo::cleanup() {
	delete file_;
}

void ActionInfo::compact_info() {
	fmt::print(
		FMT_STRING("VTF {}.{}, {} x {} x {}, {} frames, {} mipmaps, {} faces, image format {}"),
		file_->GetMajorVersion(), file_->GetMinorVersion(), file_->GetWidth(), file_->GetHeight(), file_->GetDepth(),
		file_->GetFrameCount(), file_->GetMipmapCount(), file_->GetFaceCount(),
		NAMEOF_ENUM(file_->GetFormat()));
	if (file_->GetMajorVersion() >= 7 && file_->GetMinorVersion() >= 6)
		fmt::print(FMT_STRING(", DEFLATE compression level {}\n"), file_->GetAuxCompressionLevel());
	else
		std::cout << "\n";
}
