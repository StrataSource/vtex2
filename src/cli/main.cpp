
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#include "fmt/format.h"

#include "common/vtex2_version.h"

#include "action.hpp"
#include "action_info.hpp"
#include "action_extract.hpp"
#include "action_convert.hpp"
#include "action_pack.hpp"
#include "common/util.hpp"

using namespace vtex2;

// Global flags
namespace vtex2
{
	bool verbose = false;
}

// Global list of actions
static BaseAction* s_actions[] = {new ActionInfo(), new ActionExtract(), new ActionConvert(), new ActionPack()};

static bool handle_option(int argc, int& argIndex, char** argv, ActionOption& opt);
static bool arg_compare(const char* arg, const char* argname);

[[noreturn]] static void show_help(int exitCode = 0);
[[noreturn]] static void show_action_help(BaseAction* action, int exitCode = 0);
[[noreturn]] static void show_version();

int main(int argc, char** argv) {
	bool parsingAction = false;

	BaseAction* action = nullptr;
	OptionList opts;

	for (int i = 1; i < argc; ++i) {
		const char* arg = argv[i];
		// Check if we're supposed to be parsing an action
		if (!parsingAction && *arg != '-')
			parsingAction = true;

		// Parse an action name from the command line
		if (parsingAction && !action) {
			for (auto* a : s_actions) {
				if (std::strcmp(a->get_name().c_str(), arg) == 0) {
					action = a;
					break;
				}
			}

			// If the action parsing failed, print an error & help info
			if (!action) {
				std::cerr << fmt::format("Unknown action '{}'!\n", arg);
				show_help(1);
			}

			// Duplicate list of options, we'll set them later
			opts = action->get_options();

			continue;
		}
		// Handle action-specific arguments
		else if (parsingAction) {
			// Check if this is the implicit -? or --help
			if (!std::strcmp(arg, "-?") || !std::strcmp(arg, "--help")) {
				if (!action)
					show_help(0);
				show_action_help(action, 0);
			}

			// If this doesn't start with a - or --, we'll assume it's an "end of line arg"
			if (arg[0] != '-') {
				// Find the end of line arg in the opts list and forward the rest of the args to it
				ActionOption* opt = nullptr;
				for (auto& o : opts.opts()) {
					if (o.m_endOfLine) {
						opt = &o;
						break;
					}
				}

				// Forward all following options
				if (opt) {
					std::vector<std::string> forwarded;
					for (int m = i; m < argc; ++m) {
						forwarded.push_back(argv[m]);
					}
					// @TODO: Remove this crap handling for string!
					if (opt->m_type == OptType::String)
						opt->m_value = forwarded.back();
					else
						opt->m_value = forwarded;
					opt->m_handled = true;
				}
				// Must be bad opt!
				else {
					std::cerr << fmt::format("Unexpected argument '{}'!\n", arg);
					show_help(1);
				}
				break;
			}

			// Handle this argument as a part of the action args
			for (auto& o : opts.opts()) {
				if (!std::strcmp(o.m_name[0].c_str(), arg) || !std::strcmp(o.m_name[1].c_str(), arg)) {
					if (!handle_option(argc, i, argv, o)) {
						show_help(1);
					}
					break;
				}
			}
		}
		// Handle args to the global vtex2
		else {
			if (!std::strcmp(arg, "-?") || !std::strcmp("--help", arg))
				show_help(0);
			else if (!std::strcmp(arg, "--version"))
				show_version();
		}
	}

	// No action passed?
	if (!action) {
		std::cerr << "No action specified!\n";
		show_help(1);
	}

	// Verify we have the min required args
	for (auto& o : opts.opts()) {
		if (!o.m_optional && !o.m_handled) {
			std::cerr << fmt::format("Missing required argument '{}'!\n", o.m_name[0]);
			show_action_help(action, 1);
		}
	}

	int r = action->exec(opts);
	action->cleanup();
	return r;
}

/**
 * Splits an arg by the contained =
 * --opt=something
 * -o=bruh
 * Returns true if there was separating =.
 * If false, value is not modified at all
 */
static bool split_arg(const char* arg, std::string& value) {
	auto* s = strpbrk(arg, "=");
	if (s)
		value = s;
	return !!s;
}

/**
 * Handle an action specific option
 * Return false if failed to parse
 * Options may be specified in multiple ways:
 *  --option=thing
 *  --option thing
 *  -o thing
 *  -o=thing
 */
bool handle_option(int argc, int& argIndex, char** argv, ActionOption& opt) {
	// Get next arg or default
	auto nextArg = [&](const char* def) -> const char*
	{
		if (argIndex + 1 >= argc)
			return def;
		return argv[++argIndex];
	};

	const char* arg = argv[argIndex];

	std::string valueStr;
	opt.m_handled = true;

	switch (opt.m_type) {
		case OptType::Bool:
			{
				if (split_arg(arg, valueStr)) {
					if (!str::strcasecmp(valueStr.c_str(), "false")) {
						opt.m_value = false;
						return true;
					}
					else if (!str::strcasecmp(valueStr.c_str(), "true")) {
						opt.m_value = true;
						return true;
					}
				}
				else {
					opt.m_value = true; // Empty argument string indicates true, since we're literally just a flag.
					return true;
				}
				std::cerr << fmt::format("Bad argument value '{}' for argument '{}'!\n", valueStr, arg);
				return false;
			}
		case OptType::Float:
			{
				if (!split_arg(arg, valueStr)) {
					valueStr = nextArg("");
				}

				auto val = std::strtod(valueStr.c_str(), nullptr);
				if (errno != 0) {
					std::cerr << fmt::format("Bad argument value '{}' for argument '{}'\n", valueStr, arg);
					return false;
				}
				opt.m_value = (float)val;
				return true;
			}
		case OptType::Int:
			{
				if (!split_arg(arg, valueStr)) {
					valueStr = nextArg("");
				}

				int base = 10;
				if (valueStr[0] == '0' && valueStr[1] == 'x')
					base = 16;
				auto val = std::strtol(valueStr.c_str(), nullptr, base);
				if (errno != 0) {
					std::cerr << fmt::format("Bad argument value '{}' for argument '{}'\n", valueStr, arg);
					return false;
				}
				opt.m_value = (int)val;
				return true;
			}
		case OptType::String:
			{
				if (!split_arg(arg, valueStr)) {
					valueStr = nextArg("");
				}

				// Validate choices if the requested option has them
				if (!opt.m_choices.empty()) {
					bool foundValid = false;
					for (auto& c : opt.m_choices) {
						if (valueStr == c) {
							foundValid = true;
							break;
						}
					}

					// Could not validate from list of valid choices
					if (!foundValid) {
						std::cerr << fmt::format("Bad value for option {}\nValid values are: ", arg);
						for (auto& c : opt.m_choices)
							std::cerr << fmt::format("{} ", c);
						return false;
					}
				}

				opt.m_value = valueStr;
				return true;
			}
		default:
			assert(0);
	}

	return false;
}

/**
 * Simple arg compare
 * match examples:
 *  somearg=1 and somearg
 *  somearg and somearg
 */
static bool arg_compare(const char* arg, const char* argname) {
	for (const char *a = arg, *b = argname; *a && *b; ++a, ++b) {
		if (*a == *b)
			continue;
		else if (*a == '=')
			return true;
	}
	return false;
}

static void show_help(int exitCode) {
	std::cout
		<< "USAGE: vtex2 [OPTIONS] ACTION [ARGS]...\n"
		<< "\n  Command line utility to modify, convert and show info about Valve Texture Files.\n"
		<< "\nOptions:\n";
	fmt::print("  {:<32} - Display this help text\n", "-?,--help");
	fmt::print("  {:<32} - Display version info\n", "--version");
	std::cout << "\nCommands:\n";
	for (auto& a : s_actions) {
		fmt::print("  {} - {}\n", a->get_name().c_str(), a->get_help().c_str());
	}
	std::cout << std::endl;
	exit(exitCode);
}

static void show_version() {
	fmt::print("vtex2 version {}\n", VTEX2_VERSION);
	exit(0);
}

void show_action_help(BaseAction* action, int exitCode) {
	// Find the end-of-line arg
	std::string endOfLine;
	for (auto& a : action->get_options().opts()) {
		if (a.m_endOfLine) {
			endOfLine = a.m_name[0];
			break;
		}
	}

	if (endOfLine.length())
		fmt::print("USAGE: vtex2 {} [OPTIONS] {}...\n", action->get_name(), endOfLine);
	else
		fmt::print("USAGE: vtex2 {} [OPTIONS]\n", action->get_name());

	fmt::print("\n  {}\n", action->get_help());
	std::cout << "\nOptions:\n";

	// Sort the options before display (ugly but works)
	auto sortedOpts = action->get_options().opts();
	std::sort(
		sortedOpts.begin(), sortedOpts.end(),
		[](const ActionOption& a, const ActionOption& b)
		{
			auto a1 = a.m_name[0].length() > 0 ? a.m_name[0] : a.m_name[1];
			auto b1 = b.m_name[0].length() > 0 ? b.m_name[0] : b.m_name[1];
			return str::strcasecmp(a1.c_str(), b1.c_str()) < 0;
		});

	for (auto& a : sortedOpts) {
		// Format the short and long arg names like -short,--long
		char args[256];
		snprintf(
			args, sizeof(args), "%s%s%s", a.m_name[0].c_str(),
			(a.m_name[0].length() && a.m_name[1].length()) ? "," : "", a.m_name[1].c_str());

		// Display choices (formatting hell right here!!!)
		if (!a.m_choices.empty()) {
			fmt::print("  {} [", args);
			for (int i = 0; i < a.m_choices.size(); ++i) {
				fmt::print("{}", a.m_choices[i]);
				if (i != a.m_choices.size() - 1)
					std::cout << ", ";
			}
			fmt::print("]\n{:>34} {}\n", "", a.m_desc);
		}
		// No choices, just help info
		else {
			fmt::print("  {:<32} {}\n", args, a.m_desc);
		}
	}
	std::cout << std::endl;
	exit(exitCode);
}
