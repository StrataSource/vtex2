
#include "action.hpp"
#include "common/image.hpp"

namespace VTFLib
{
	class CVTFFile;
}

namespace vtex2
{

	/**
	 * A simple action to display info about a VTF file
	 */
	class ActionPack : public BaseAction {
	public:
		std::string get_name() const override {
			return "pack";
		}
		std::string get_help() const override;
		const OptionList& get_options() const override;
		int exec(const OptionList& opts) override;
		void cleanup() override;

	private:
		using path = std::filesystem::path;

		bool pack_mrao(const path& outpath, const path& m, const path& r, const path& ao, const OptionList& opts);
		bool pack_normal(const path& outpath, const path& n, const path& h, const OptionList& opts);
		bool save_vtf(const path& out, const imglib::ImageData_t& data,  const OptionList& opts, bool normal);

		VTFLib::CVTFFile* file_ = nullptr;
	};

} // namespace vtex2
