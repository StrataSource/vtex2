
#include "action_info.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"

#include "VTFLib.h"

using namespace vtex2;

namespace opts {
	static int all;
	static int file;
}

std::string ActionInfo::get_help() const {
	return "Displays info about a VTF file";
}

const std::vector<ActionOption>& ActionInfo::get_options() const {
	static std::vector<ActionOption> opts;
	if (opts.empty()) {
		opts.push_back( ActionOption {
				.name = {"--all", "-a"},
				.type = OptType::Bool,
				.value = false,
				.desc = "Display all detailed info about a VTF",
				.optional = true,
				.endOfLine = false,
		});
		opts::all = opts.size()-1;
		
		opts.push_back( ActionOption {
				.name = {"file", ""},
				.type = OptType::String,
				.value = "",
				.desc = "File to display info about",
				.optional = false,
				.endOfLine = true,
		});
		opts::file = opts.size()-1;
	};
	return opts;
}

int ActionInfo::exec(const std::vector<ActionOption>& opts) {
	
	auto file = opts[opts::file].get<std::string>();
	auto details = opts[opts::all].get<bool>(false);
	
	// Load off disk
	std::uint8_t* buf = nullptr;
	auto numBytes = read_file(file, buf);
	
	if (numBytes == 0 || !buf) {
		fprintf(stderr, "Could not open file '%s'!\n", file.c_str());
		delete [] buf;
		return 1;
	}
	
	// Load VTF with vtflib
	file_ = new VTFLib::CVTFFile();
	if (!file_->Load(buf, numBytes, false)) {
		fprintf(stderr, "Failed to load VTF '%s': %s\n", file.c_str(),
			vlGetLastError());
		delete file_;
		return 1;
	}
	
	auto fmt = file_->GetFormat();
	
	printf("VTF Version %d.%d\n", file_->GetMajorVersion(), file_->GetMinorVersion());
	printf("Image format: %s\n", ImageFormatToString(fmt));
	printf("Dimensions (WxHxD): %d x %d x %d\n", file_->GetWidth(), file_->GetHeight(), file_->GetDepth());
	printf("%d frame(s), %d face(s), %d mipmaps\n", file_->GetFrameCount(), file_->GetFaceCount(), file_->GetMipmapCount());
	
	if (details) {
		vlUInt dataSize;
		(file_->GetResourceData(VTF_RSRC_CRC, dataSize));
		if (auto* crcPtr = file_->GetResourceData(VTF_RSRC_CRC, dataSize)) {
			auto crc = *static_cast<std::uint32_t*>(crcPtr);
			printf("Source CRC: %d (0x%X)\n", crc, crc);
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
	
	printf("%d KiB image data (%.2f MiB)\n", file_->GetSize() / 1024, file_->GetSize() / (1024.f*1024.f));
	
	delete file_;
	return 0;
}
