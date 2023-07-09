
#include "vtftools.hpp"
#include "image.hpp"

#include "VTFLib.h"

#undef min
#undef max

using namespace VTFLib;

bool vtf::resize(const CVTFFile* srcFile, int newWidth, int newHeight, CVTFFile* file) {

	const int frameCount = srcFile->GetFrameCount();
	const int faceCount = srcFile->GetFaceCount();
	const int sliceCount = srcFile->GetDepth();
	const int srcWidth = srcFile->GetWidth();
	const int srcHeight = srcFile->GetHeight();

	// Choose the best channel type for this
	auto fmtinfo = srcFile->GetImageFormatInfo(srcFile->GetFormat());
	auto maxBpp = std::max(
		std::max(fmtinfo.uiAlphaBitsPerPixel, fmtinfo.uiBlueBitsPerPixel),
		std::max(fmtinfo.uiGreenBitsPerPixel, fmtinfo.uiRedBitsPerPixel));

	// Choose the best image format type for this, so we dont lose image depth!
	// We'll just switch between RGBA8/16/32F for simplicity, although it will require 33% more mem.
	imglib::ChannelType type = imglib::ChannelType::UInt8;
	if (maxBpp > 16)
		type = imglib::ChannelType::Float;
	else if (maxBpp > 8)
		type = imglib::ChannelType::UInt16;

	// Create a working buffer to hold our data when we want to convert it
	auto convSize = CVTFFile::ComputeImageSize(newWidth, newHeight, 1, file->GetFormat());
	auto convBuffer = std::make_unique<vlByte[]>(convSize);

	// Resize all base level mips for each frame, face and slice.
	for (vlUInt uiFrame = 0; uiFrame < frameCount; ++uiFrame) {
		for (vlUInt uiFace = 0; uiFace < faceCount; ++uiFace) {
			for (vlUInt uiSlice = 0; uiSlice < sliceCount; ++uiSlice) {
				// Get & resize the data now
				void* data = srcFile->GetData(uiFrame, uiFace, uiSlice, 0);
				void* newData = nullptr;
				if (!imglib::resize(data, &newData, type, 4, srcWidth, srcHeight, newWidth, newHeight)) {
					return false;
				}

				// Load the image data into the dest now
				file->SetData(uiFrame, uiFace, uiSlice, 0, static_cast<vlByte*>(newData));

				free(newData);
			}
		}
	}

	return true;
}