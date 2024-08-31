
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "action_pack.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"
#include "common/pack.hpp"

#include "VTFLib.h"

#include "fmt/format.h"

// Windows junk
#undef min
#undef max

using namespace vtex2;
using namespace VTFLib;
using namespace pack;

namespace opts
{
	static int file;
	static int mrao;
	static int normal;
	static int nmap, hmap;
	static int rmap, aomap, mmap, tmtex;
	static int width, height, mips;
	static int mconst, rconst, aoconst, hconst;
	static int toDX;
	static int quiet;
} // namespace opts

std::string ActionPack::get_help() const {
	return "Packs images into a MRAO or normal+height map";
}

const OptionList& ActionPack::get_options() const {
	static OptionList opts;
	if (opts.empty()) {

		opts::mrao = opts.add(
			ActionOption()
				.long_opt("--mrao")
				.short_opt("-mrao")
				.type(OptType::Bool)
				.value(false)
				.help("Create MRAO map"));

		opts::normal = opts.add(
			ActionOption()
				.long_opt("--normal")
				.short_opt("-n")
				.type(OptType::Bool)
				.value(false)
				.help("Create packed normal+height map"));

		opts::nmap = opts.add(
			ActionOption()
				.long_opt("--normal-map")
				.short_opt("-nmap")
				.type(OptType::String)
				.value("")
				.help("Normal map to pack"));

		opts::hmap = opts.add(
			ActionOption()
				.long_opt("--height-map")
				.short_opt("-hmap")
				.type(OptType::String)
				.value("")
				.help("Height map to pack"));

		opts::tmtex = opts.add(
			ActionOption()
				.long_opt("--tint-mask")
				.short_opt("-tmask")
				.type(OptType::String)
				.value("")
				.help("Tint mask texture to use [MRAO only]"));

		opts::aomap = opts.add(
			ActionOption()
				.long_opt("--ao-map")
				.short_opt("-aomap")
				.type(OptType::String)
				.value("")
				.help("AO map to pack [MRAO only]"));

		opts::mmap = opts.add(
			ActionOption()
				.long_opt("--metalness-map")
				.short_opt("-mmap")
				.type(OptType::String)
				.value("")
				.help("Metalness map to pack [MRAO only]"));

		opts::rmap = opts.add(
			ActionOption()
				.long_opt("--roughness-map")
				.short_opt("-rmap")
				.type(OptType::String)
				.value("")
				.help("Roughness map to pack [MRAO only]"));

		opts::width = opts.add(
			ActionOption()
				.type(OptType::Int)
				.value(-1)
				.long_opt("--width")
				.short_opt("-w")
				.help("Width of output image. If set to -1, autodetect from input maps"));

		opts::height = opts.add(
			ActionOption()
				.type(OptType::Int)
				.value(-1)
				.long_opt("--height")
				.short_opt("-h")
				.help("Height (dimension) of output image. If set to -1, autodetect from input maps"));

		opts::mips = opts.add(
			ActionOption()
				.type(OptType::Int)
				.value(10)
				.long_opt("--mips")
				.short_opt("-m")
				.help("Number of mipmaps for output image"));

		opts::file = opts.add(
			ActionOption()
				.long_opt("--output")
				.short_opt("-o")
				.type(OptType::String)
				.value(false)
				.end_of_line(true)
				.help("Output file path"));

		opts::rconst = opts.add(
			ActionOption()
				.long_opt("--roughness-const")
				.short_opt("-rc")
				.type(OptType::Float)
				.value(1.0f)
				.help("If no roughness map is specified, fill roughness value with this constant value"));

		opts::mconst = opts.add(
			ActionOption()
				.long_opt("--metalness-const")
				.short_opt("-mc")
				.type(OptType::Float)
				.value(0.0f)
				.help("If no metalness map is specified, fill metalness value with this constant value"));

		opts::aoconst = opts.add(
			ActionOption()
				.long_opt("--ao-const")
				.short_opt("-aoc")
				.type(OptType::Float)
				.value(1.0f)
				.help("If no AO map is specified, fill AO value with this constant value"));

		opts::hconst = opts.add(
			ActionOption()
				.long_opt("--height-const")
				.short_opt("-hc")
				.type(OptType::Float)
				.value(0.0f)
				.help("If no height map is specified, fill height value with this constant value"));

		opts::toDX = opts.add(
			ActionOption()
				.long_opt("--opengl")
				.short_opt("-gl")
				.value(false)
				.type(OptType::Bool)
				.help("Treat the incoming normal map as a OpenGL normal map"));

		opts::quiet = opts.add(
			ActionOption()
				.long_opt("--quiet")
				.short_opt("-q")
				.value(false)
				.type(OptType::Bool)
				.help("Silence output messages that aren't errors")
		);
	};
	return opts;
}

int ActionPack::exec(const OptionList& opts) {

	const auto file = opts.get<std::string>(opts::file);

	const auto isNormal = opts.get<bool>(opts::normal);
	const auto isMRAO = opts.get<bool>(opts::mrao);
	const auto outpath = opts.get<std::string>(opts::file);

	if (isNormal) {
		const auto n = opts.get<std::string>(opts::nmap);
		const auto h = opts.get<std::string>(opts::hmap);
		return pack_normal(outpath, n, h, opts) ? 0 : 1;
	}
	else if (isMRAO) {
		const auto r = opts.get<std::string>(opts::rmap);
		const auto m = opts.get<std::string>(opts::mmap);
		const auto ao = opts.get<std::string>(opts::aomap);
		const auto tm = opts.get<std::string>(opts::tmtex);
		return pack_mrao(outpath, m, r, ao, tm, opts) ? 0 : 1;
	}
	else {
		std::cerr << "No action specified: please specify --mrao or --normal!\n";
	}

	return 1;
}

//
// Wrapper to load an image and display error if it can't be loaded
//
static std::shared_ptr<imglib::Image> load_image(const std::filesystem::path& path) {

	auto img = imglib::Image::load(path); // Force UInt8 for packed images
	if (!img)
		std::cerr << fmt::format("Could not load image '{}'\n", path.string());
	return img;
}

//
// Ugly function to determine out size based on inputs
//
template <int N>
static void determine_size(int* w, int* h, const std::shared_ptr<imglib::Image> (&datas)[N]) {
	for (int i = 0; i < N; ++i) {
		if (!datas[i])
			continue;
		if (w)
			*w = std::max(*w, datas[i]->width());
		if (h)
			*h = std::max(*h, datas[i]->height());
	}
}

//
// Resize images if required and converts too!
//
static bool resize_if_required(const std::shared_ptr<imglib::Image>& image, int w, int h) {
	if (!image)
		return true;

	// @TODO: For now we're just going to force 8 bit per channel.
	//  Sometimes we do get 16bpc images, mainly for height data, but we're cramming that into a RGBA8888 texture
	//  anyways. It'd be best to eventually support RGBA16F normals for instances where you need precise height data.
	if (image->type() != imglib::ChannelType::UInt8) {
		if (!image->convert(imglib::ChannelType::UInt8)) {
			std::cerr << "Failed to convert image\n";
			return false;
		}
	}

	if (image->width() == w && image->height() == h)
		return true;
	return image->resize(w, h);
}

//
// Pack an MRAO map
//
bool ActionPack::pack_mrao(
	const std::filesystem::path& outpath, const path& metalnessFile, const path& roughnessFile, const path& aoFile,
	const path& tmask, const OptionList& opts) {
	auto clampw = opts.get<int>(opts::width);
	auto clamph = opts.get<int>(opts::height);

	const bool usingR = !roughnessFile.empty();
	const bool usingM = !metalnessFile.empty();
	const bool usingAO = !aoFile.empty();
	const bool usingTMask = !tmask.empty();
	const float rconst = opts.get<float>(opts::rconst);
	const float mconst = opts.get<float>(opts::mconst);
	const float aoconst = opts.get<float>(opts::aoconst);

	std::shared_ptr<imglib::Image> roughnessData, aoData, metalnessData, tmaskData;

	// Load all images
	if (usingR && !(roughnessData = load_image(roughnessFile)))
		return false;
	if (usingAO && !(aoData = load_image(aoFile)))
		return false;
	if (usingM && !(metalnessData = load_image(metalnessFile)))
		return false;
	if (usingTMask && !(tmaskData = load_image(tmask)))
		return false;

	// Determine width and height if not set
	int w = -1, h = -1;
	determine_size(&w, nullptr, {roughnessData, aoData, metalnessData, tmaskData});
	determine_size(nullptr, &h, {roughnessData, aoData, metalnessData, tmaskData});

	if (w <= 0 || h <= 0) {
		if (clampw <= 0 || clamph <= 0) {
			std::cerr << ((clampw <= 0) ? "-w" : "-h") << " is required to pack this image.\n";
			return false;
		}
		w = clampw;
		h = clamph;
	}

	// Resize images if required
	if (!resize_if_required(roughnessData, w, h))
		return false;
	if (!resize_if_required(metalnessData, w, h))
		return false;
	if (!resize_if_required(aoData, w, h))
		return false;
	if (!resize_if_required(tmaskData, w, h))
		return false;

	// Packing config
	pack::ChannelPack_t pack[] = {
		{.srcChan = 0,
		 .dstChan = 0,
		 .srcData = metalnessData ? metalnessData->data<uint8_t>() : nullptr,
		 .comps = metalnessData ? metalnessData->channels() : 1,
		 .constant = mconst},
		{.srcChan = 0,
		 .dstChan = 1,
		 .srcData = roughnessData ? roughnessData->data<uint8_t>() : nullptr,
		 .comps = roughnessData ? roughnessData->channels() : 1,
		 .constant = rconst},
		{.srcChan = 0,
		 .dstChan = 2,
		 .srcData = aoData ? aoData->data<uint8_t>() : nullptr,
		 .comps = aoData ? aoData->channels() : 1,
		 .constant = aoconst},
		{
			.srcChan = 0,
			.dstChan = 3,
			.srcData = tmaskData ? tmaskData->data<uint8_t>() : nullptr,
			.comps = tmaskData ? tmaskData->channels() : 1,
			.constant = 1,
		}};

	// tint mask texture is last in channels list, so skip it if we're not given a tint mask
	const auto numSrcChans = usingTMask ? util::ArraySize(pack) : util::ArraySize(pack) - 1;
	const auto numDstChans = usingTMask ? 4 : 3; // RGBA when using tint mask texture in mrao.w

	// Finally, pack the darn thing
	auto outImage = pack::pack_image(numDstChans, pack, numSrcChans, w, h);
	if (!outImage) {
		std::cerr << "Packing failed!\n";
		return false;
	}

	// Free up some mem
	if (roughnessData)
		roughnessData->clear();
	if (aoData)
		aoData->clear();
	if (metalnessData)
		metalnessData->clear();
	if (tmaskData)
		tmaskData->clear();

	// If user requested clamp, do that now
	if (clampw > 0 || clamph > 0) {
		if (!(clampw > 0 && clamph > 0)) {
			std::cerr << "Both -w/--width and -h/--height must be specified to clamp the image.\n";
			return false;
		}
		if (!outImage->resize(clampw, clamph)) {
			std::cerr << "Image resize failed\n";
			return false;
		}
	}

	bool success = true;
	if ((success = save_vtf(outpath, outImage, opts, false)))
		if (!opts.get<bool>(opts::quiet))
			std::cout << fmt::format("Finished {} ({} KiB)\n", outpath.string(), file_->GetSize() / 1024);
	return success;
}

//
// Pack height into normal
//
bool ActionPack::pack_normal(
	const std::filesystem::path& outpath, const path& normalFile, const path& heightFile, const OptionList& opts) {
	auto clampw = opts.get<int>(opts::width);
	auto clamph = opts.get<int>(opts::height);
	const bool isGL = opts.get<bool>(opts::toDX);

	const bool usingH = !heightFile.empty();
	const float hconst = opts.get<float>(opts::hconst);

	if (normalFile.empty()) {
		std::cerr << "--normal-map must be specified!\n";
		return false;
	}

	std::shared_ptr<imglib::Image> heightData, normalData;

	// Load all images
	if (usingH && !(heightData = load_image(heightFile)))
		return false;
	if (!(normalData = load_image(normalFile)))
		return false;

	// Determine width and height if not set
	int w = -1, h = -1;
	determine_size(&w, nullptr, {normalData, heightData});
	determine_size(nullptr, &h, {normalData, heightData});

	if (w <= 0 || h <= 0) {
		if (clampw <= 0 || clamph <= 0) {
			std::cerr << ((clampw <= 0) ? "-w" : "-h") << " is required to pack this image.\n";
			return false;
		}
		w = clampw;
		h = clamph;
	}

	// Resize images if required
	if (!resize_if_required(normalData, w, h))
		return false;
	if (!resize_if_required(heightData, w, h))
		return false;

	// Convert normal to DX if necessary
	if (isGL)
		normalData->process(imglib::PROC_GL_TO_DX_NORM);

	// Packing config
	pack::ChannelPack_t pack[] = {
		{.srcChan = 0,
		 .dstChan = 0,
		 .srcData = normalData->data<uint8_t>(),
		 .comps = normalData->channels(),
		 .constant = 0.0f},
		{.srcChan = 1,
		 .dstChan = 1,
		 .srcData = normalData->data<uint8_t>(),
		 .comps = normalData->channels(),
		 .constant = 0.0f},
		{.srcChan = 2,
		 .dstChan = 2,
		 .srcData = normalData->data<uint8_t>(),
		 .comps = normalData->channels(),
		 .constant = 0.0f},
		{.srcChan = 0,
		 .dstChan = 3,
		 .srcData = heightData->data<uint8_t>(),
		 .comps = heightData->channels(),
		 .constant = hconst}};

	std::shared_ptr<imglib::Image> outImage;

	// Finally, pack the darn thing
	bool success = true;
	if (!(outImage = pack::pack_image(4, pack, util::ArraySize(pack), w, h))) {
		success = false;
		std::cerr << "Packing failed!\n";
		return false;
	}

	// Free up some mem
	heightData->clear();
	normalData->clear();

	// If user requested clamp, do that now
	if (clampw > 0 || clamph > 0) {
		if (!(clampw > 0 && clamph > 0)) {
			std::cerr << "Both -w/--width and -h/--height must be specified to clamp the image.\n";
			return false;
		}
		if (!outImage->resize(clampw, clamph)) {
			std::cerr << "Image resize failed\n";
			return false;
		}
	}

	success = save_vtf(outpath, outImage, opts, true);

	if (success && !opts.get<bool>(opts::quiet))
		std::cout << fmt::format("Finished {} ({} KiB)\n", outpath.string(), file_->GetSize() / 1024);
	return success;
}

//
// Save resulting image data to disk
// Automatically determines the format to use on save based on channels in data
//
bool ActionPack::save_vtf(
	const std::filesystem::path& out, const std::shared_ptr<imglib::Image>& image, const OptionList& opts,
	bool normal) {
	file_ = new CVTFFile();

	SVTFInitOptions initOpts{};
	initOpts.ImageFormat = image->channels() == 3 ? IMAGE_FORMAT_RGB888 : IMAGE_FORMAT_RGBA8888;
	initOpts.nMipMaps = 10;
	initOpts.uiFaces = initOpts.uiFrames = 1;
	initOpts.uiHeight = image->height();
	initOpts.uiWidth = image->width();
	initOpts.uiSlices = 1;
	if (!file_->Init(initOpts)) {
		std::cerr << fmt::format("Error while saving VTF: {}\n", util::get_last_vtflib_error());
		return false; // Cleanup done in cleanup()
	}
	file_->SetData(0, 0, 0, 0, image->data<vlByte>());
	file_->SetFlag(TEXTUREFLAGS_NORMAL, normal);

	file_->GenerateMipmaps(MIPMAP_FILTER_CATROM, false);
	return file_->Save(out.string().c_str());
}

void ActionPack::cleanup() {
	delete file_;
}
