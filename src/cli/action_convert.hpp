
#include <filesystem>

#include "action.hpp"

namespace VTFLib {
	class CVTFFile;
}

namespace vtex2 {
	
	/**
	 * Extract image data from a VTF and put it in a
	 * generic image file
	 */
	class ActionConvert : public BaseAction {
	public:
		std::string get_name() const override { return "convert"; }
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;
		
		bool process_file(const OptionList& opts, const std::filesystem::path& srcFile, const std::filesystem::path& outPath);
		bool add_image_data(const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file);
	};
	
}
