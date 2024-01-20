#include "DependencyGraph.h"
#include "Dependency.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>

// If you are on Windows Server 2019 or Windows 10 version 1803 or later, your OS already set up and ready to use.
// Default path is C : \Windows\System32\curl.exe

//curl - L - H "Accept: application/json" - H "Authorization: Bearer <YOUR-TOKEN>" - H "X-GitHub-Api-Version: 2022-11-28" https://api.github.com/repos/TannerLow/C-Machine-Learning/contents/
//curl - L - H "Accept: application/json" - H "X-GitHub-Api-Version: 2022-11-28" https ://api.github.com/repos/TannerLow/C-Machine-Learning/contents/
//
//curl - L - H "Accept: application/vnd.github+json" - H "Authorization: Bearer <YOUR-TOKEN>" - H "X-GitHub-Api-Version: 2022-11-28" https ://api.github.com/repos/TannerLow/C-Machine-Learning/releases/RELEASE_ID

//curl -s -L -H "Accept:application/octet-stream" https://github.com/TannerLow/C-Debug-Helper/releases/download/v1.0/CDebugHelper.lib --output tmp.file

void curlDriver() {
	const std::string cmd = "curl -L -H \"Accept:application/vnd.github.raw\" -H \"X-GitHub-Api-Version:2022-11-28\" https://api.github.com/repos/TannerLow/C-Machine-Learning/contents/stdsafety.h";
	const std::string tmpFileName = "tmp_github_result.deleteme";

	int exitCode = system((cmd + " > " + tmpFileName).c_str());
	if (exitCode) {
		// Do something
	}

	std::vector<std::string> contents;
	std::string buffer;

	std::ifstream file(tmpFileName, std::ifstream::binary);
	while (std::getline(file, buffer)) {
		contents.push_back(buffer);
	}

	file.close();

	for (auto line : contents) {
		std::cout << line << std::endl;
	}

	// unchecked error
	remove(tmpFileName.c_str());
}

void localDependenciesTest() {
	Dependency::localReposDir = "test/local";
	DependencyGraph dependencyGraph("test/local/dependencies.cfg");
	dependencyGraph.print();
}

void remoteDependenciesTest() {
	DependencyGraph dependencyGraph("test/remote/dependencies.cfg");
	dependencyGraph.print();
}

void remoteLocalMixTest() {
	Dependency::localReposDir = "test/mix";
	DependencyGraph dependencyGraph("test/mix/dependencies.cfg");
	dependencyGraph.print();
}

#include <stdio.h>
#include "DependencyGraph.h"
#include <filesystem>
#include "Dependency.h"
#include <stack>

namespace fs = std::filesystem;

enum class ErrorCode {
	SUCCESS,
	TOO_MANY_CMD_ARGS,
	EMPTY_OR_MISSING_CONFIG
};

int errorCode(ErrorCode code) {
	return (int)code;
}

#include "MinizWrapper.h"
#include "Makefile.h"
#include "Template.h"

// Gather dependencies based on dependency config. Will recursively gather all 
// dependency configs into /DependencyConfigs
int defaultBuild() {
	std::cout << make::makefile << std::endl;

	// Future feature idea: Make a command line arg to specify your own makefile template
	Template makefileTemplate(make::makefile);

	std::cout << "Keys:" << std::endl;
	for (auto key : makefileTemplate.getKeys()) {
		std::cout << key << std::endl;
	}

	// dynamically load these based on a config or command prompt input
	std::map<std::string, std::string> makefileInserts;
	makefileInserts["modules"] = ". device matrix util";
	makefileInserts["includes"] = "-I \"./include\" -I \"./include/OpenCL/Nvidia\"";

	makefileTemplate.fillTemplate(makefileInserts);

	std::ofstream makefile("makefile");
	if (makefile.is_open()) {
		makefile << makefileTemplate.getFilledTemplate();
		makefile.close();
	}

//	PackageCache\TannerLow\C-Matrix-Library\v1.0
	//fs::create_directories("./include/dependencies");

	//std::cout << "unzip ret code: " 
	//	<< zip::unzip("PackageCache/TannerLow/C-Matrix-Library/v1.0/include.zip", "include/dependencies/") 
	//	<< std::endl;

	//return 0;

	// should move this build config parsing to a separate file or something
	std::string debugLibsDir;
	std::string releaseLibsDir;

	std::ifstream buildConfig("build.cfg");
	if (!buildConfig.is_open()) {
		return -1;
	}

	std::string line;
	while (std::getline(buildConfig, line)) {
		size_t colon = line.find(":");
		if (colon != std::string::npos) {
			if (line.substr(0, colon) == "debug") {
				debugLibsDir = line.substr(colon + 2);
			}
			else if (line.substr(0, colon) == "release") {
				releaseLibsDir = line.substr(colon + 2);
			}
		}
	}

	buildConfig.close();


	std::string packageCacheDir = Dependency::localReposDir;
	if (!fs::is_directory(packageCacheDir) || !fs::exists(packageCacheDir)) {
		if (!fs::create_directory(packageCacheDir)) {
			return -1;
		}
	}

	const char* config = "dependencies.cfg";

	DependencyGraph dependencyGraph(config);

	std::stack<const Dependency*> dependenciesToPull = dependencyGraph.getTopologicalOrder();
	while (!dependenciesToPull.empty()) {
		const Dependency* dependencyToPull = dependenciesToPull.top();
		std::cout << "Pulling " << dependencyToPull->repo << std::endl;
		dependencyToPull->pullFiles("include/dependencies", releaseLibsDir + "/", debugLibsDir + "/");
		dependenciesToPull.pop();
	}

	return errorCode(ErrorCode::SUCCESS);
}

#include "Curl.h"

int newCurlTest() {
	cbs::CurlFileArguments fileArguments1;
	cbs::CurlFileArguments fileArguments2;
	cbs::CurlCommand command;
	
	fileArguments1.fileUrl = "https://github.com/TannerLow/C-Debug-Helper/releases/download/v1.0/dependencies.cfg";
	fileArguments1.outputFilepath = "DependencyConfigs/dependencies.cfg";
	fileArguments2.fileUrl = "https://github.com/TannerLow/C-Debug-Helper/releases/download/v1.0/include.zip";
	fileArguments2.outputFilepath = "Headers/include.zip";

	command.options.push_back("-L");
	command.headers.push_back("Accept:application/octet-stream");
	command.fileArguments.push_back(fileArguments1);
	command.fileArguments.push_back(fileArguments2);
	
	if (cbs::curl(command)) {
		std::cout << "Curl returned 0 woot" << std::endl;
	}
	else {
		return -1;
	}
	return errorCode(ErrorCode::SUCCESS);
}

int unexpectedArgCount(int argc) {
	printf("[Error] Too many arguments provided: %d arguments provided\n", argc);
	return errorCode(ErrorCode::TOO_MANY_CMD_ARGS);
}

int main(const int argc, const char** argv) {
	//curlDriver();
	//localDependenciesTest();
	//remoteDependenciesTest();
	//remoteLocalMixTest();

	switch (argc) {
	//case 1: return newCurlTest();
	case 1 : return defaultBuild();
	default: return unexpectedArgCount(argc);
	}

	return 0;
}