
#include "action.hpp"

namespace VTFLib {
	class CVTFFile;
}

namespace vtex2 {
	
	/**
	 * Extract image data from a VTF and put it in a
	 * generic image file
	 */
	class ActionExtract : public BaseAction {
	public:
		std::string get_name() const override { return "extract"; }
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;
		
	private:
		VTFLib::CVTFFile* file_ = nullptr;
	};
	
}
