
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <array>
#include <vector>

#include "common/types.hpp"

namespace vtex2 {
	
	enum class OptType {
		Float,			// Float type, may be scientific notation, normal or hex
		Int,			// Integer type, 1e6, 0x399, etc
		Bool,			// A simple switch
		Vec2,			// 2 component float vector
		Vec3,			// 3 component float vector
		Vec4,			// 4 component float vector
		String,			// Arbitrary string
		StringArr,		// Multiple strings
	};
	
	using OptValue = std::variant<bool, int, float, vec2, vec3, vec4, std::string, std::vector<std::string>>;
	
	/**
	 * Defines an option that an action may have
	 */
	struct ActionOption {
		std::array<std::string, 2> name;		// Array containing short opt name, and long opt name. At least one must be specified
		OptType type;			// Data type of the option
		OptValue value;			// Default value of the option. When passed into exec(), this will be the "real" value
		std::string desc;		// Description of the option, shown in help dialog
		bool optional = false;	// If true, this option may be excluded from the command line
		bool endOfLine = false;	// If true, this is an "end of line" argument which may accept multiple values
		bool handled = false;	// Internal use only...
		
		/**
		 * Helper to get values
		 */
		template<class T>
		T get(const T& def = T()) const {
			auto* a = std::get_if<T>(&this->value);
			if (!a)
				return def;
			return *a;
		}
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
		virtual const std::vector<ActionOption>& get_options() const = 0;
		
		/**
		 * @brief Executes the action
		 * @param opts List of options
		 * @return int A return code. If not 0, the application will stop processing any further actions
		 * after this one returns. If not 0, this return code will effectively be bubbled up
		 * to main() and be the exit status of the program
		 */
		virtual int exec(const std::vector<ActionOption>& opts) = 0;
	};
	
}
