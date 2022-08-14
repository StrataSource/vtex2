
#include <filesystem>

#include "action.hpp"

namespace VTFLib
{
	class CVTFFile;
}

namespace vtex2
{

	/**
	 * Modify existing VTF in-place
	 */
	class ActionModify : public BaseAction {
	public:
		std::string get_name() const override {
			return "modify";
		}
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;

		bool process_file(const OptionList& opts, const std::filesystem::path& srcFile) const;
	};

} // namespace vtex2
