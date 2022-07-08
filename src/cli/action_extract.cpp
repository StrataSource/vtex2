
#include "action_extract.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"
#include "common/strtools.hpp"
#include "common/image.hpp"

#include "VTFLib.h"

using namespace vtex2;

namespace opts {
	static int output;
	static int file;
	static int format;
}

std::string ActionExtract::get_help() const {
	return "Displays info about a VTF file";
}

const std::vector<ActionOption>& ActionExtract::get_options() const {
	static std::vector<ActionOption> opts;
	if (opts.empty()) {
		opts.push_back( ActionOption {
				.name = {"--output", "-o"},
				.type = OptType::String,
				.value = "",
				.desc = "File to place the output in",
				.optional = false,
				.endOfLine = false,
		});
		opts::output = opts.size()-1;
		
		opts.push_back( ActionOption {
				.name = {"--format", "-f"},
				.type = OptType::String,
				.value = "",
				.desc = "Output format to use (png, jpeg, tga)",
				.optional = true,
				.endOfLine = false,
		});
		opts::format = opts.size()-1;
		
		opts.push_back( ActionOption {
				.name = {"file", ""},
				.type = OptType::String,
				.value = "",
				.desc = "VTF file to convert",
				.optional = false,
				.endOfLine = true,
		});
		opts::file = opts.size()-1;
	};
	return opts;
}

int ActionExtract::exec(const std::vector<ActionOption>& opts) {
	
	auto file = opts[opts::file].get<std::string>();
	auto output = opts[opts::output].get<std::string>();
	auto format = opts[opts::format].get<std::string>();
	
	// Load off disk
	std::uint8_t* buf = nullptr;
	auto numBytes = util::read_file(file, buf);
	auto bufCleanup = util::cleanup([&buf]{
		delete [] buf;
	});
	
	
	if (numBytes == 0 || !buf) {
		fprintf(stderr, "Could not open file '%s'!\n", file.c_str());
		delete [] buf;
		return 1;
	}
	
	// Create new file & load it with vtflib
	file_ = new VTFLib::CVTFFile();
	auto vtfCleanup = util::cleanup([this]{
		delete file_;
	});
	
	if (!file_->Load(buf, numBytes, false)) {
		fprintf(stderr, "Failed to load VTF '%s': %s\n", file.c_str(),
			vlGetLastError());
		return 1;
	}
	
	// Determine format based on output file extension 
	std::string targetFormatName = str::get_ext(output.c_str());
	if (!format.empty())
		targetFormatName = format;
	auto targetFmt = imglib::image_get_format(targetFormatName.c_str());
	
	// Ensure target file format is valid
	if (targetFmt == imglib::FileFormat::None) {
		fprintf(stderr, "Could not determine file format from file '%s'. To explicitly choose a format, pass --format.\n",
			output.c_str());
		return 1;
	}
	
	const int w = file_->GetWidth();
	const int h = file_->GetHeight();
	
	auto formatInfo = file_->GetImageFormatInfo(file_->GetFormat());
	const int comps = 4; // Convered output file will always have alpha
	
	
	// Allocate buffer for image data
	vlByte* imageData = nullptr;
	auto imageDataClean = util::cleanup([imageData] {
		free(imageData);
	});
	
	// Only supported format for Hdr is 32-bit RGBA - everything else will be squashed down into RGB formats
	bool destIsFloat = (targetFmt == imglib::Hdr);
	bool ok = false;
	if (destIsFloat) {
		imageData = static_cast<vlByte*>(malloc(w * h * comps * sizeof(float)));
		ok = VTFLib::CVTFFile::Convert(file_->GetData(0, 0, 0, 0), imageData, w, h, file_->GetFormat(), 
			IMAGE_FORMAT_RGBA32323232F);
	}
	else {
		imageData = static_cast<vlByte*>(malloc(w * h * comps * sizeof(uint8_t)));
		ok = VTFLib::CVTFFile::ConvertToRGBA8888(file_->GetData(0, 0, 0, 0), imageData, w, h, file_->GetFormat());
	}
	
	if (!ok) {
		fprintf(stderr, "Could not convert image format '%s' -> '%s'!\n", ImageFormatToString(file_->GetFormat()),
			destIsFloat ? "RGBA32323232F" : "RGBA8888");
		return 1;
	}
	
	imglib::ImageData_t data {};
	data.info.comps = comps;
	data.info.frames = 1; // @TODO: Multi-frame support
	data.info.h = h;
	data.info.w = w;
	data.info.type = destIsFloat ? imglib::Float : imglib::UInt8;
	data.data = imageData;
	
	if (!imglib::image_save(data, output.c_str(), targetFmt)) {
		fprintf(stderr, "Could not save image to '%s'!\n", output.c_str());
		return 1;
	}
	
	return 0;
}
