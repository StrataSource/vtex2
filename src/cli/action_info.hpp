
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
		const std::vector<ActionOption>& get_options() const override;
		int exec(const std::vector<ActionOption>& opts) override;
		void cleanup() override;
	private:
		VTFLib::CVTFFile* file_ = nullptr;
	};
	
}
