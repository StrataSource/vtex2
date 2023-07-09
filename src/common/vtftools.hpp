
#pragma once

namespace VTFLib
{
	class CVTFFile;
}

namespace vtf
{
	/**
	 * Resize a VTF, and put the resulting image data into file
	 * @param srcFile file to draw data from
	 * @param newWidth New width
	 * @param newHeight New height
	 * @param file File to write data to
	 * @returns true if the resize passed
	 */
	bool resize(const VTFLib::CVTFFile* srcFile, int newWidth, int newHeight, VTFLib::CVTFFile* file);
} // namespace vtf