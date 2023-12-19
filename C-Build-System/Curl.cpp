#include "Curl.h"

bool cbs::curl(const CurlCommand curlCommand) {
	std::string command = "curl ";

	for (const auto& option : curlCommand.options) {
		command += " " + option;
	}

	for (const auto& header : curlCommand.headers) {
		command += " -H \"" + header + "\"";
	}

	for (const auto& fileArgument : curlCommand.fileArguments) {
		command += " " + fileArgument.fileUrl;
		command += " --output " + fileArgument.outputFilepath.string();
	}

	int exitCode = system(command.c_str());
	return exitCode == 0;
}