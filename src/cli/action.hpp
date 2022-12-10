
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <array>
#include <vector>
#include <list>

#include "common/types.hpp"

namespace vtex2
{

	/**
	 * Global flags
	 */
	extern bool verbose;

	enum class OptType {
		Float,	   // Float type, may be scientific notation, normal or hex
		Int,	   // Integer type, 1e6, 0x399, etc
		Bool,	   // A simple switch
		String,	   // Arbitrary string
		StringArr, // Multiple strings
	};

	using OptValue = std::variant<bool, int, float, vec2, vec3, vec4, std::string, std::vector<std::string>>;

	/**
	 * Defines an option that an action may have
	 */
	struct ActionOption {
		std::array<std::string, 2> m_name; // Array containing short opt name, and long opt name. At least one must be
										   // specified
		OptType m_type;					   // Data type of the option
		OptValue m_value;		  // Default value of the option. When passed into exec(), this will be the "real" value
		std::string m_desc;		  // Description of the option, shown in help dialog
		bool m_optional = true;	  // If true, this option may be excluded from the command line
		bool m_endOfLine = false; // If true, this is an "end of line" argument which may accept multiple values
		bool m_handled = false;	  // Internal use only...
		int m_numArgs = 1;
		std::vector<std::string> m_choices; // Valid choices - only implemented for strings for now!!!

		ActionOption& long_opt(const std::string& a) {
			m_name[1] = a;
			return *this;
		}
		ActionOption& short_opt(const std::string& a) {
			m_name[0] = a;
			return *this;
		}
		ActionOption& type(OptType t) {
			m_type = t;
			return *this;
		}
		ActionOption& value(const OptValue& v) {
			m_value = v;
			return *this;
		}
		ActionOption& help(const std::string& h) {
			m_desc = h;
			return *this;
		}
		ActionOption& required(bool b) {
			m_optional = !b;
			return *this;
		}
		ActionOption& end_of_line(bool b) {
			m_endOfLine = b;
			return *this;
		}
		ActionOption& num_args(int n) {
			m_numArgs = n;
			return *this;
		}
		ActionOption& metavar(const std::string& meta) {
			m_name[0] = meta;
			return *this;
		}
		ActionOption& choices(const std::initializer_list<std::string>& choices) {
			for (auto& c : choices)
				m_choices.push_back(c);
			return *this;
		}

		/**
		 * Helper to get values
		 */
		template <class T>
		T get(const T& def = T()) const {
			auto* a = std::get_if<T>(&this->m_value);
			if (!a)
				return def;
			return *a;
		}

		bool provided() const {
			return m_handled;
		}
	};

	/**
	 * Option list
	 * Effectively a wrapper around list
	 */
	class OptionList {
	public:
		auto add(const ActionOption& opt) {
			m_opts.push_back(opt);
			return m_opts.size() - 1;
		}

		const ActionOption* find(const std::string& name) const {
			for (auto& o : m_opts)
				if (name == o.m_name[0] || name == o.m_name[1])
					return &o;
			return nullptr;
		}

		ActionOption& get(int index) {
			return m_opts[index];
		}

		bool has(int index) const {
			return m_opts[index].provided();
		}

		template <class T>
		const T get(int index) const {
			return m_opts[index].get<T>();
		}

		bool empty() const {
			return m_opts.empty();
		}

		const auto& opts() const {
			return m_opts;
		}

		auto& opts() {
			return m_opts;
		}

	private:
		std::vector<ActionOption> m_opts;
	};

	/**
	 * Base class for all action types
	 *
	 * Actions define operations that vtex2 can perform,
	 * and they're invoked via the command line.
	 * They may have options as well, passed in from the
	 * command line.
	 * By default, each action supports a `-h` or `--help`
	 * option.
	 *
	 */
	class BaseAction {
	public:
		/**
		 * @brief Returns the name of this action
		 */
		virtual std::string get_name() const = 0;

		/**
		 * @brief Returns a help string.
		 * This string will be displayed to the user if they type `vtex2 myaction -h`
		 * Following this string, the option helps will be printed
		 */
		virtual std::string get_help() const = 0;

		/**
		 * @brief Returns a list of options.
		 * These options will not be modified and instead act as a template that is used to
		 * create a set of actual options with real values.
		 */
		virtual const OptionList& get_options() const = 0;

		/**
		 * @brief Executes the action
		 * @param opts List of options
		 * @return int A return code. If not 0, the application will stop processing any further actions
		 * after this one returns. If not 0, this return code will effectively be bubbled up
		 * to main() and be the exit status of the program
		 */
		virtual int exec(const OptionList& opts) = 0;

		/**
		 * @brief Do any cleanup
		 * This is called after exec
		 */
		virtual void cleanup() = 0;
	};

} // namespace vtex2
