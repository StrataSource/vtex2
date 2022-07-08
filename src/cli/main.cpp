
#include <cstring>
#include <cassert>
#include <cstdlib>

#include "action.hpp"
#include "action_info.hpp"
#include "action_extract.hpp"
#include "common/util.hpp"

using namespace vtex2;

[[noreturn]] static void show_help(int exitCode = 0);

// Global list of actions
static BaseAction* s_actions[] =
{
	new ActionInfo(),
	new ActionExtract()
};

bool handle_option(int argc, int &argIndex, char** argv, ActionOption& opt);

int main(int argc, char** argv) {
	bool parsingAction = false;
	
	BaseAction* action = nullptr;
	std::vector<ActionOption> opts;
	
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
				show_help(0); // @TODO
			}
			
			// If this doesn't start with a - or --, we'll assume it's an "end of line arg"
			if (arg[0] != '-') {
				// Find the end of line arg in the opts list and forward the rest of the args to it
				ActionOption* opt = nullptr;
				for (auto& o : opts) {
					if (o.endOfLine) {
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
					if(opt->type == OptType::String)
						opt->value = forwarded.back();
					else
						opt->value = forwarded;
					opt->handled = true;
				}
				// Must be bad opt!
				else {
					fprintf(stderr, "Unexpected argument '%s'!\n", arg);
					show_help(1);
				}
				break;
			}
			
			// Handle this argument as a part of the action args
			for (auto& o : opts) {
				if (!std::strcmp(o.name[0].c_str(), arg) || !std::strcmp(o.name[1].c_str(), arg)) {
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
		show_help(0);
	}
	
	// Verify we have the min required args
	for (auto& o : opts) {
		if (!o.optional && !o.handled) {
			fprintf(stderr, "Missing required argument '%s'!\n", o.name[0].c_str());
			show_help(1);
		}
	}
	
	return action->exec(opts);
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
	opt.handled = true;
	
	switch(opt.type) {
		case OptType::Bool:
		{
			if (split_arg(arg, valueStr)) {
				if (!strcasecmp(valueStr.c_str(), "false")) {
					opt.value = false;
					return true;
				}
				else if (!strcasecmp(valueStr.c_str(), "true")) {
					opt.value = true;
					return true;
				}
			}
			else {
				opt.value = true; // Empty argument string indicates true, since we're literally just a flag.
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
			opt.value = (float)val;
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
			opt.value = (int)val;
			return true;
		}
		case OptType::String:
		{
			if (!split_arg(arg, valueStr)) {
				valueStr = nextArg("");
			}
			
			opt.value = valueStr;
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

void show_help(int exitCode) {
	printf("USAGE: vtex2 [OPTIONS...] ACTION [ACTION OPTIONS...]\n");
	printf("\nAvailable actions:\n");
	for (auto& a : s_actions) {
		printf("\t%s - %s\n", a->get_name().c_str(), a->get_help().c_str());
	}
	
	exit(exitCode);
}
