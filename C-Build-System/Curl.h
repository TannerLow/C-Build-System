#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace cbs {
	namespace fs = std::filesystem;

	struct CurlFileArguments {
		std::string fileUrl;
		fs::path outputFilepath;
	};

	struct CurlCommand {
		std::vector<std::string> options;
		std::vector<std::string> headers;
		std::vector<CurlFileArguments> fileArguments;
	};

	// true is curl command succeeds
	// Output contents can be found at each of the fileArguments.outputFilepath
	bool curl(const CurlCommand curlCommand);
}