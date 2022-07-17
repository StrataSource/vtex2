
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <algorithm>

#include "action.hpp"
#include "action_info.hpp"
#include "action_extract.hpp"
#include "common/util.hpp"

using namespace vtex2;

// Global flags
namespace vtex2 {
	bool verbose = false;
}

// Global list of actions
static BaseAction* s_actions[] =
{
	new ActionInfo(),
	new ActionExtract()
};

static bool handle_option(int argc, int &argIndex, char** argv, ActionOption& opt);
static bool arg_compare(const char* arg, const char* argname);

[[noreturn]] static void show_help(int exitCode = 0);
[[noreturn]] static void show_action_help(BaseAction* action, int exitCode = 0);



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
				printf("Unknown action '%s'!\n", arg);
				show_help(1);
			}
			
			// Duplicate list of options, we'll set them later
			opts = action->get_options();
			
			continue;
		}
		// Handle action-specific arguments
		else if (parsingAction) {
			// Check if this is the implicit -h or --help
			if (!std::strcmp(arg, "-h") || !std::strcmp(arg, "--help")) {
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
					if(opt->m_type == OptType::String)
						opt->m_value = forwarded.back();
					else
						opt->m_value = forwarded;
					opt->m_handled = true;
				}
				// Must be bad opt!
				else {
					fprintf(stderr, "Unexpected argument '%s'!\n", arg);
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
			if (!std::strcmp(arg, "-h") || !std::strcmp("--help", arg))
				show_help(0);
		}
	}
	
	// No action passed? 
	if (!action) {
		fprintf(stderr, "No action specified!\n");
		show_help(1);
	}
	
	// Verify we have the min required args
	for (auto& o : opts.opts()) {
		if (!o.m_optional && !o.m_handled) {
			fprintf(stderr, "Missing required argument '%s'!\n", o.m_name[0].c_str());
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
	for (const char* s = arg; *s; s++) {
		if(*s == '"' || *s == '\'')
			break;
		// Unquoted equal!!!
		if (*s == '=') {
			// Extract portion following = 
			char comp2[256];
			std::strncpy(comp2, s+1, sizeof(comp2));
			comp2[sizeof(comp2)-1] = 0;
			value = comp2;
			return true;
		}
	}
	return false;
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
bool handle_option(int argc, int &argIndex, char** argv, ActionOption& opt) {	
	// Get next arg or default
	auto nextArg = [&](const char* def) -> const char* {
		if (argIndex+1 >= argc)
			return def;
		return argv[++argIndex];
	};
	
	const char* arg = argv[argIndex];
	
	std::string valueStr;
	opt.m_handled = true;
	
	switch(opt.m_type) {
		case OptType::Bool:
		{
			if (split_arg(arg, valueStr)) {
				if (!strcasecmp(valueStr.c_str(), "false")) {
					opt.m_value = false;
					return true;
				}
				else if (!strcasecmp(valueStr.c_str(), "true")) {
					opt.m_value = true;
					return true;
				}
			}
			else {
				opt.m_value = true; // Empty argument string indicates true, since we're literally just a flag.
				return true;
			}
			fprintf(stderr, "Bad argument value '%s' for argument '%s'!\n", valueStr.c_str(), arg);
			return false;
		}
		case OptType::Float:
		{
			if (!split_arg(arg, valueStr)) {
				valueStr = nextArg("");
			}
			
			auto val = std::strtod(valueStr.c_str(), nullptr);
			if (errno != 0) {
				fprintf(stderr, "Bad argument value '%s' for argument '%s'\n", valueStr.c_str(), arg);
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
				fprintf(stderr, "Bad argument value '%s' for argument '%s'\n", valueStr.c_str(), arg);
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
					fprintf(stderr, "Bad value for option %s\nValid values are: ");
					for (auto& c : opt.m_choices)
						fprintf(stderr, "%s ", c.c_str());
					return false;
				}
			}
			
			opt.m_value = valueStr;
			return true;
		}
		case OptType::Vec2:
		{
			assert(0);
			break;
		}
		case OptType::Vec3:
		{
			assert(0);
			break;
		}
		case OptType::Vec4:
		{
			assert(0);
			break;
		}
		default: assert(0);
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
	for (const char* a = arg, *b = argname; *a && *b; ++a, ++b) {
		if (*a == *b)
			continue;
		else if (*a == '=')
			return true;
	}
	return false;
}

void show_help(int exitCode) {
	printf("USAGE: vtex2 [OPTIONS] ACTION [ARGS]...\n");
	printf("\n  Command line utility to modify, convert and show info about Valve Texture Files.\n");
	printf("\nOptions:\n");
	printf("\nCommands:\n");
	for (auto& a : s_actions) {
		printf("  %s - %s\n", a->get_name().c_str(), a->get_help().c_str());
	}
	printf("\n");
	exit(exitCode);
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
		printf("USAGE: vtex2 %s [OPTIONS] %s...\n", action->get_name().c_str(), endOfLine.c_str());
	else
		printf("USAGE: vtex2 %s [OPTIONS]\n", action->get_name().c_str());

	printf("\n  %s\n", action->get_help().c_str());
	printf("\nOptions:\n");

	// Sort the options before display (ugly but works)
	auto sortedOpts = action->get_options().opts();
	std::sort(sortedOpts.begin(), sortedOpts.end(), [](const ActionOption& a, const ActionOption& b) {
		auto a1 = a.m_name[0].length() > 0 ? a.m_name[0] : a.m_name[1];
		auto b1 = b.m_name[0].length() > 0 ? b.m_name[0] : b.m_name[1];
		return str::strcasecmp(a1.c_str(), b1.c_str()) < 0;
	});
	
	for (auto& a : sortedOpts) {
		// Format the short and long arg names like -short,--long
		char args[256];
		snprintf(args, sizeof(args), "%s%s%s", a.m_name[0].c_str(), 
			(a.m_name[0].length()&&a.m_name[1].length()) ? "," : "", a.m_name[1].c_str());
			
		// Display choices (formatting hell right here!!!)
		if (!a.m_choices.empty()) {
			printf("  %s", args);
			printf(" [");
			for (int i = 0; i < a.m_choices.size(); ++i) {
				printf("%s", a.m_choices[i].c_str());
				if (i != a.m_choices.size()-1)
					printf(", ");
			}
			printf("]\n%-23s%s\n", "", a.m_desc.c_str());
		}
		// No choices, just help info
		else {
			printf("  %-20s %s\n", args, a.m_desc.c_str());
		}
	}
	printf("\n");
	exit(exitCode);
}
