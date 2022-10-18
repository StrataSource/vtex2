
#include <iostream>
#include <unordered_map>

#include "action_pack.hpp"
#include "common/util.hpp"
#include "common/enums.hpp"
#include "common/pack.hpp"

#include "VTFLib.h"

#include "fmt/format.h"

using namespace vtex2;
using namespace VTFLib;
using namespace pack;

namespace opts
{
	static int file;
	static int mrao;
	static int normal;
	static int nmap, hmap;
	static int rmap, aomap, mmap;
	static int width, height, mips;
	static int batch, recurse;
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

		opts::batch = opts.add(
			ActionOption()
				.long_opt("--batch")
				.short_opt("-b")
				.type(OptType::Bool)
				.value(false)
				.help("Pack files in a directory"));

		opts::recurse = opts.add(
			ActionOption()
				.long_opt("--recursive")
				.short_opt("-r")
				.type(OptType::Bool)
				.value(false)
				.help("Pack files in a directory recursively"));

		opts::nmap = opts.add(
			ActionOption()
				.long_opt("--normal-map")
				.type(OptType::String)
				.value("")
				.help("Normal map to pack"));

		opts::hmap = opts.add(
			ActionOption()
				.long_opt("--height-map")
				.type(OptType::String)
				.value("")
				.help("Height map to pack"));

		opts::aomap = opts.add(
			ActionOption()
				.long_opt("--ao-map")
				.type(OptType::String)
				.value("")
				.help("AO map to pack"));

		opts::mmap = opts.add(
			ActionOption()
				.long_opt("--metalness-map")
				.type(OptType::String)
				.value("")
				.help("Metalness map to pack"));

		opts::rmap = opts.add(
			ActionOption()
				.long_opt("--roughness-map")
				.type(OptType::String)
				.value("")
				.help("Roughness map to pack"));

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
				.help("Height of output image. If set to -1, autodetect from input maps"));

		opts::mips = opts.add(
			ActionOption()
				.type(OptType::Int)
				.value(10)
				.long_opt("--mips")
				.short_opt("-m")
				.help("Number of mipmaps for output image"));

		opts::file = opts.add(
			ActionOption()
				.long_opt("--outfile")
				.short_opt("-o")
				.type(OptType::String)
				.value(false)
				.end_of_line(true)
				.help("Output file name"));
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
		return pack_mrao(outpath, m, r, ao, opts) ? 0 : 1;
	}

	return 1;
}

//
// Wrapper to load an image and display error if it can't be loaded
//
static bool load_image(const std::filesystem::path& path, imglib::ImageData_t& out) {
	auto* fp = imglib::image_begin(path);
	if (!fp) {
		std::cerr << fmt::format("Could not load image '{}'\n", path.string());
		return false;
	}

	auto d = imglib::image_load(fp);
	if (!d.data) {
		imglib::image_end(fp);
		std::cerr << fmt::format("Could not load image '{}'\n", path.string());
		return false;
	}
	
	imglib::image_end(fp);
	out = d;
	return true;
}

//
// Ugly function to determine out size based on inputs
//
template<int N>
static void determine_size(int* w, int* h, const imglib::ImageData_t (&datas)[N]) {
	for (int i = 0; i < N; ++i) {
		if (!datas[i].data)
			continue;
		if (w)
			*w = std::max(*w, datas[i].info.w);
		if (h)
			*h = std::max(*h, datas[i].info.h);
	}
}

//
// Resize images if required
//
static void resize_if_required(imglib::ImageData_t& data, int w, int h) {
	if (data.info.w == w && data.info.h == h)
		return;
	assert(imglib::resize(data, w, h));
}

//
// Pack an MRAO map
//
bool ActionPack::pack_mrao(const std::filesystem::path& outpath, const path& metalnessFile, const path& roughnessFile, const path& aoFile, const OptionList& opts) {
	auto w = opts.get<int>(opts::width);
	auto h = opts.get<int>(opts::height);

	if (roughnessFile.empty() || aoFile.empty() || metalnessFile.empty()) {
		std::cerr << "--roughness-map, --ao-map and --metalness-map must all be specified!\n";
		return false;
	}

	imglib::ImageData_t roughnessData {}, aoData {}, metalnessData {};
	util::cleanup cleanup([&]{
		imglib::image_free(roughnessData);
		imglib::image_free(aoData);
		imglib::image_free(metalnessData);
	});

	// Load all images
	if (!load_image(roughnessFile, roughnessData))
		return false;
	if (!load_image(aoFile, aoData))
		return false;
	if (!load_image(metalnessFile, metalnessData))
		return false;

	// Determine width and height if not set
	if (w <= 0)
		determine_size(&w, nullptr, {roughnessData, aoData, metalnessData});
	if (h <= 0)
		determine_size(nullptr, &h, {roughnessData, aoData, metalnessData});

	if (w <= 0 || h <= 0) {
		std::cerr << "-h and -w are required to pack!\n";
		return false;
	}

	// Resize images if required
	resize_if_required(roughnessData, w, h);
	resize_if_required(metalnessData, w, h);
	resize_if_required(aoData, w, h);

	// Packing config
	pack::ChannelPack_t pack[] = {
		{
			.srcChan = 0,
			.dstChan = 0,
			.srcData = static_cast<uint8_t*>(metalnessData.data),
			.comps = metalnessData.info.comps,
			.constant = 0.0f
		},
		{
			.srcChan = 0,
			.dstChan = 1,
			.srcData = static_cast<uint8_t*>(roughnessData.data),
			.comps = roughnessData.info.comps,
			.constant = 0.0f
		},
		{
			.srcChan = 0,
			.dstChan = 2,
			.srcData = static_cast<uint8_t*>(aoData.data),
			.comps = aoData.info.comps,
			.constant = 0.0f
		}
	};

	imglib::ImageData_t outImage {};

	// Finally, pack the darn thing
	bool success = true;
	if (!pack::pack_image(outImage, 3, pack, util::ArraySize(pack), w, h)) {
		success = false;
		std::cerr << "Packing failed!\n";
		return false;
	}

	// Free up some mem
	imglib::image_free(roughnessData);
	imglib::image_free(aoData);
	imglib::image_free(metalnessData);

	success = save_vtf(outpath, outImage, opts, false);

	imglib::image_free(outImage);
	if (success)
		std::cout << fmt::format("Finished processing {}\n", outpath.string());
	return success;
}

//
// Pack height into normal
//
bool ActionPack::pack_normal(const std::filesystem::path& outpath, const path& normalFile, const path& heightFile, const OptionList& opts) {
	auto w = opts.get<int>(opts::width);
	auto h = opts.get<int>(opts::height);

	if (normalFile.empty() || heightFile.empty()) {
		std::cerr << "--height-map and --normal-map must all be specified!\n";
		return false;
	}

	imglib::ImageData_t heightData {}, normalData {};
	util::cleanup cleanup([&]{
		imglib::image_free(heightData);
		imglib::image_free(normalData);
	});

	// Load all images
	if (!load_image(heightFile, heightData))
		return false;
	if (!load_image(normalFile, normalData))
		return false;

	// Determine width and height if not set
	if (w <= 0)
		determine_size(&w, nullptr, {normalData, heightData});
	if (h <= 0)
		determine_size(nullptr, &h, {normalData, heightData});

	if (w <= 0 || h <= 0) {
		std::cerr << "-h and -w are required to pack!\n";
		return false;
	}

	// Resize images if required
	resize_if_required(normalData, w, h);
	resize_if_required(heightData, w, h);

	// Packing config
	pack::ChannelPack_t pack[] = {
		{
			.srcChan = 0,
			.dstChan = 0,
			.srcData = static_cast<uint8_t*>(normalData.data),
			.comps = normalData.info.comps,
			.constant = 0.0f
		},
		{
			.srcChan = 1,
			.dstChan = 1,
			.srcData = static_cast<uint8_t*>(normalData.data),
			.comps = normalData.info.comps,
			.constant = 0.0f
		},
		{
			.srcChan = 2,
			.dstChan = 2,
			.srcData = static_cast<uint8_t*>(normalData.data),
			.comps = normalData.info.comps,
			.constant = 0.0f
		},
		{
			.srcChan = 0,
			.dstChan = 3,
			.srcData = static_cast<uint8_t*>(heightData.data),
			.comps = heightData.info.comps,
			.constant = 0.0f
		}
	};

	imglib::ImageData_t outImage {};

	// Finally, pack the darn thing
	bool success = true;
	if (!pack::pack_image(outImage, 4, pack, util::ArraySize(pack), w, h)) {
		success = false;
		std::cerr << "Packing failed!\n";
		return false;
	}

	// Free up some mem
	imglib::image_free(heightData);
	imglib::image_free(normalData);

	success = save_vtf(outpath, outImage, opts, true);

	imglib::image_free(outImage);
	if (success)
		std::cout << fmt::format("Finished processing {}\n", outpath.string());
	return success;
}

//
// Save resulting image data to disk
//
bool ActionPack::save_vtf(const std::filesystem::path& out, const imglib::ImageData_t& data, const OptionList& opts, bool normal) {
	file_ = new CVTFFile();
	
	SVTFInitOptions initOpts {};
	initOpts.ImageFormat = data.info.comps == 3 ? IMAGE_FORMAT_RGB888 : IMAGE_FORMAT_RGBA8888;
	initOpts.nMipMaps = 10;
	initOpts.uiFaces = initOpts.uiFrames = 1;
	initOpts.uiHeight = data.info.h;
	initOpts.uiWidth = data.info.w;
	initOpts.uiSlices = 1;
	if (!file_->Init(initOpts)) {
		std::cerr << fmt::format("Error while saving VTF: {}\n", vlGetLastError());
		return false; // Cleanup done in cleanup()
	}
	file_->SetData(0, 0, 0, 0, static_cast<vlByte*>(data.data));
	file_->SetFlag(TEXTUREFLAGS_NORMAL, normal);
	
	file_->GenerateMipmaps(MIPMAP_FILTER_CATROM, false);
	return file_->Save(out.string().c_str());
}


void ActionPack::cleanup() {
	delete file_;
}
