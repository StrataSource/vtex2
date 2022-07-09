
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
	static int mip;
}

std::string ActionExtract::get_help() const {
	return "Displays info about a VTF file";
}

const OptionList& ActionExtract::get_options() const {
	static OptionList opts;
	if (opts.empty()) {
		opts::output = opts.add( ActionOption()
				.short_opt("-o")
				.long_opt("--output")
				.type(OptType::String)
				.value("")
				.help("File to place the output in")
				.required(true)
		);
		
		opts::format = opts.add( ActionOption()
				.short_opt("-f")
				.long_opt("--format")
				.type(OptType::String)
				.value("")
				.help("Output format to use (png, jpeg, tga)")
		);
		
		opts::file = opts.add( ActionOption()
				.metavar("file")
				.type(OptType::String)
				.value("")
				.help("VTF file to convert")
				.required(true)
				.end_of_line(true)
		);
		
		opts::mip = opts.add( ActionOption()
				.short_opt("-m")
				.long_opt("--mip")
				.type(OptType::Int)
				.value(0)
				.help("Mipmap to extract from image")
		);
	};
	return opts;
}

int ActionExtract::exec(const OptionList& opts) {
	
	auto file = opts.get(opts::file).get<std::string>();
	auto output = opts.get(opts::output).get<std::string>();
	auto format = opts.get(opts::format).get<std::string>();
	auto mip = opts.get(opts::mip).get<int>();
	
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
	if (!file_->Load(buf, numBytes, false)) {
		fprintf(stderr, "Failed to load VTF '%s': %s\n", file.c_str(),
			vlGetLastError());
		return 1;
	}
	
	// Validate mipmap selection
	if (mip > file_->GetMipmapCount()) {
		fprintf(stderr, "Selected mip %d exceeds the total mip count of the image: %d\n", 
			mip, file_->GetMipmapCount());
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
	
	vlUInt w, h, d;
	file_->ComputeMipmapDimensions(file_->GetWidth(), file_->GetHeight(), file_->GetDepth(), mip, w, h, d);
	
	auto formatInfo = file_->GetImageFormatInfo(file_->GetFormat());
	int comps = 4; // Convered output file will always have alpha
	
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
		ok = VTFLib::CVTFFile::Convert(file_->GetData(0, 0, 0, mip), imageData, w, h, file_->GetFormat(), 
			IMAGE_FORMAT_RGBA32323232F);
	}
	else {
		comps = 3;
		imageData = static_cast<vlByte*>(malloc(w * h * comps * sizeof(uint8_t)));
		ok = VTFLib::CVTFFile::Convert(file_->GetData(0, 0, 0, mip), imageData, w, h, file_->GetFormat(), IMAGE_FORMAT_RGB888);
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

void ActionExtract::cleanup() {
	delete file_;
}
