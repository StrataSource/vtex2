
#include "action_info.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

#include "VTFLib.h"

using namespace vtex2;

namespace opts {
	static int all;
	static int file;
	static int resources;
}

std::string ActionInfo::get_help() const {
	return "Displays info about a VTF file";
}

const OptionList& ActionInfo::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::all = opts.add( ActionOption()
				.long_opt("--all")
				.short_opt("-a")
				.type(OptType::Bool)
				.value(false)
				.help("Display all detailed info about a VTF")
		);
		
		opts::resources = opts.add( ActionOption()
				.long_opt("--resources")
				.short_opt("-r")
				.type(OptType::Bool)
				.value(false)
				.help("List all resource entries in the VTF")
		);
		
		opts::file = opts.add( ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("VTF file to process")
				.end_of_line(true)
				.required(true)
		);
	};
	return opts;
}

int ActionInfo::exec(const OptionList& opts) {
	
	const auto file = opts.get(opts::file).get<std::string>();
	const auto details = opts.get(opts::all).get<bool>();
	const auto resources = opts.get(opts::resources).get<bool>() || details;
	
	// Load off disk
	std::uint8_t* buf = nullptr;
	auto numBytes = util::read_file(file, buf);
	auto bufCleanup = util::cleanup([&]{
		delete [] buf;
	});
	
	if (numBytes == 0 || !buf) {
		fprintf(stderr, "Could not open file '%s'!\n", file.c_str());
		return 1;
	}
	
	// Load VTF with vtflib
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(buf, numBytes, false)) {
		fprintf(stderr, "Failed to load VTF '%s': %s\n", file.c_str(),
			vlGetLastError());
		return 1;
	}
	
	auto fmt = file_->GetFormat();
	
	// Basic compact info mode
	if (!details) {
		compact_info();
		return 0;
	}
	
	printf("VTF Version %d.%d\n", file_->GetMajorVersion(), file_->GetMinorVersion());
	printf("Image format: %s\n", ImageFormatToString(fmt));
	printf("Dimensions (WxHxD): %d x %d x %d\n", file_->GetWidth(), file_->GetHeight(), file_->GetDepth());
	printf("%d frame(s), %d face(s), %d mipmaps\n", file_->GetFrameCount(), file_->GetFaceCount(), file_->GetMipmapCount());
	
	if (details) {
		vlUInt dataSize;
		if (auto* crcPtr = file_->GetResourceData(VTF_RSRC_CRC, dataSize)) {
			auto crc = *static_cast<std::uint32_t*>(crcPtr);
			printf("Source CRC: 0x%X\n", crc);
		}
		else 
			printf("Source CRC: None\n");
			
		// Display list of texture flags
		auto flags = TextureFlagsToStringVector(file_->GetFlags());
		printf("Flags: 0x%X\n", file_->GetFlags());
		for (auto& fl : flags) {
			printf("    %s\n", fl.c_str());
		}
		
		printf("Bumpscale: %f\n", file_->GetBumpmapScale());
		vlSingle x,y,z;
		file_->GetReflectivity(x, y, z);
		printf("Reflectivity: (%f %f %f)\n", x, y, z);
	}
	
	if (resources) {
		const auto rsrcCount = file_->GetResourceCount();
		rsrcCount > 0 ? printf("Resource entries:\n") : printf("No resource entries\n");
		
		for (auto i = 0; i < rsrcCount; ++i) {
			auto type = file_->GetResourceType(i);
			vlUInt sz = 0;
			file_->GetResourceData(type, sz);
			printf("    0x%X (%c%c%c) - %u bytes (%.1f KiB)\n", type, type & 0xFF, (type>>8) & 0xFF, (type>>16) & 0xFF, sz, sz/1024.f);
		}
	}
	
	printf("%d KiB image data (%.2f MiB)\n", file_->GetSize() / 1024, file_->GetSize() / (1024.f*1024.f));
	
	return 0;
}

void ActionInfo::cleanup() {
	delete file_;
}

void ActionInfo::compact_info() {
	printf("VTF %d.%d, %d x %d x %d, %d frames, %d mipmaps, %d faces, image format %s\n",
		file_->GetMajorVersion(), file_->GetMinorVersion(),
		file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), file_->GetFrameCount(),
		file_->GetMipmapCount(), file_->GetFaceCount(), ImageFormatToString(file_->GetFormat()));
}
