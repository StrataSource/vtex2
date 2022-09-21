
#include <filesystem>

#include "action.hpp"
#include "VTFLib.h"

namespace VTFLib
{
	class CVTFFile;
}

namespace vtex2
{

	/**
	 * Extract image data from a VTF and put it in a
	 * generic image file
	 */
	class ActionConvert : public BaseAction {
	public:
		std::string get_name() const override {
			return "convert";
		}
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;

		bool set_properties(VTFLib::CVTFFile* file);

		bool process_file(
			const OptionList& opts, const std::filesystem::path& srcFile, const std::filesystem::path& outPath);
		
		bool add_image_data(
			const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file, VTFImageFormat format, bool create);
		
		bool add_image_data_raw(VTFLib::CVTFFile* file, const void* data, VTFImageFormat format, VTFImageFormat dataFormat, int w, int h, bool create);
		
		bool add_vtf_image_data(VTFLib::CVTFFile* srcImage, VTFLib::CVTFFile* file, VTFImageFormat format);
		
		VTFLib::CVTFFile* init_from_file(const std::filesystem::path& src, VTFLib::CVTFFile* file, VTFImageFormat newFormat);

	private:
		int m_mips = 10;
		int m_width = -1;
		int m_height = -1;
		const OptionList* m_opts = nullptr;
	};

} // namespace vtex2
