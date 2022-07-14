
#include "action.hpp"


namespace VTFLib {
	class CVTFFile;
}

namespace vtex2 {
	
	/**
	 * A simple action to display info about a VTF file
	 */
	class ActionInfo : public BaseAction {
	public:
		std::string get_name() const override { return "info"; }
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;
		
	private:
		void compact_info();
	
		VTFLib::CVTFFile* file_ = nullptr;
	};
	
}
