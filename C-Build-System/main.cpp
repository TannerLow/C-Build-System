#include "DependencyGraph.h"
#include "Dependency.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <regex>

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
	SUCCESS = 0,
	TOO_FEW_CMD_ARGS,
	EMPTY_OR_MISSING_CONFIG,
	UNRESOLVED_ACTION,
	INVALID_PROJECT_NAME,
	UNIMPLEMENTED,
	COULD_NOT_CREATE_DIRECTORY,
	CONFLICTING_PROJECT_DIRECTORY,
	MAKEFILE_TEMPLATE_ERROR,
	MAKEFILE_CREATION_ERROR,
	DEPENDENCIES_CONFIG_CREATION_ERROR,
	INVALID_MAKEFILE_INPUT,
	COULD_NOT_CREATE_FILE,
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

	//std::stack<const Dependency*> dependenciesToPull = dependencyGraph.getTopologicalOrder();
	//while (!dependenciesToPull.empty()) {
	//	const Dependency* dependencyToPull = dependenciesToPull.top();
	//	std::cout << "Pulling " << dependencyToPull->repo << std::endl;
	//	dependencyToPull->pullFiles("include/dependencies", releaseLibsDir + "/", debugLibsDir + "/");
	//	dependenciesToPull.pop();
	//}

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

int unexpectedArgCount() {
	printf("[Error] Too few arguments provided.\nExpected use:\n\tcbuild <action> [<arguments> ...]\n");
	return errorCode(ErrorCode::TOO_FEW_CMD_ARGS);
}

int unresolvedAction(const std::string action) {
	printf("[Error] Unknown action provided: %s provided\n", action.c_str());
	printf("[Info] Valid actions are init, pull, and package\n");
	return errorCode(ErrorCode::UNRESOLVED_ACTION);
}

bool isValidProjectName(const std::string projectName) {
	std::regex pattern("[a-zA-Z0-9\\-_]+");
	return std::regex_match(projectName, pattern);
}

int init(const std::vector<std::string> arguments) {
// Input validation
	if (arguments.size() < 2) {
		printf("[Error] No project name provided.\nExpected use:\n\tcbuild init <project_name>\n");
		return errorCode(ErrorCode::TOO_FEW_CMD_ARGS);
	}

	std::string projectName = arguments[1];
	if (!isValidProjectName(projectName)) {
		printf("[Error] Desired project name is invalid. Name must contain only letters, numbers, dash (-), or underscore (_).\n");
		return errorCode(ErrorCode::INVALID_PROJECT_NAME);
	}

// Setup new project structure
	fs::path projectRoot(projectName);
	if (fs::exists(projectRoot)) {
		printf("[Error] Project name conflicts with existing directory.\n");
		return errorCode(ErrorCode::CONFLICTING_PROJECT_DIRECTORY);
	}

	if (!fs::create_directory(projectRoot)) {
		printf("[Error] Failed to create project directory.\n");
		return errorCode(ErrorCode::COULD_NOT_CREATE_DIRECTORY);
	}

	fs::path includeDir = projectRoot / "include";
	if (!fs::create_directory(includeDir)) {
		printf("[Error] Failed to create \"includes\" directory.\n");
		return errorCode(ErrorCode::COULD_NOT_CREATE_DIRECTORY);
	}

	fs::path srcDir = projectRoot / "src";
	if (!fs::create_directory(srcDir)) {
		printf("[Error] Failed to create \"src\" directory.\n");
		return errorCode(ErrorCode::COULD_NOT_CREATE_DIRECTORY);
	}

	fs::path testDir = projectRoot / "test";
	if (!fs::create_directory(testDir)) {
		printf("[Error] Failed to create \"test\" directory.\n");
		return errorCode(ErrorCode::COULD_NOT_CREATE_DIRECTORY);
	}

	fs::path starterCode = testDir / "main.c";
	std::ofstream starterCodeFile(starterCode);
	if (!starterCodeFile.is_open()) {
		printf("[Error] Failed to create \"main.c\".\n");
		return errorCode(ErrorCode::COULD_NOT_CREATE_FILE);
	}
	starterCodeFile << "\nint main() {\n\treturn 0;\n}\n";
	starterCodeFile.close();

	// Future feature idea: Make a command line arg to specify your own makefile template
	Template makefileTemplate(make::makefile);

	// Debug printing
	//std::cout << "Keys:" << std::endl;
	//for (auto key : makefileTemplate.getKeys()) {
	//	std::cout << key << std::endl;
	//}

	// Ask for project parameters needed for makefile
	printf("Which platform will this project compile for? [Default: x86_64]\n>");
	std::string platform;
	std::getline(std::cin, platform);
	if (platform.empty()) {
		platform = "x86_64";
	}

	printf("Enter the release library file name? [Default: %s.lib]\n>", projectName.c_str());
	std::string libraryName;
	std::getline(std::cin, libraryName);
	if (libraryName.empty()) {
		libraryName = projectName + ".lib";
	}

	printf("Enter the debug library file name? [Default: %s-d.lib]\n>", projectName.c_str());
	std::string debugLibraryName;
	std::getline(std::cin, debugLibraryName);
	if (debugLibraryName.empty()) {
		debugLibraryName = projectName + "-d.lib";
	}

	// Dynamically load these based on a config or command prompt input
	std::map<std::string, std::string> makefileInserts;
	makefileInserts["includes"    ] = "-I \"./include\" -I \"./include/dependencies\"";
	makefileInserts["platform"    ] = platform; // build.cfg, could be asked on init and auto populate the build.cfg
	makefileInserts["libs"        ] = ""; // can get later with dependencies.cfg // "-l \"CDebugHelper\" -l \"OpenCL_nvidia\"";
	makefileInserts["debugLibs"   ] = ""; // same as previous // "-l \"CDebugHelper-d\" -l \"OpenCL_nvidia\"";
	makefileInserts["library"     ] = libraryName; // ask at init // "CMatrixLib.lib";
	makefileInserts["debugLibrary"] = debugLibraryName; // ask at init // "CMatrixLib-d.lib";

	if (!makefileTemplate.fillTemplate(makefileInserts)) {
		printf("[Error] Failed to create makefile from template.\n");
		return errorCode(ErrorCode::MAKEFILE_TEMPLATE_ERROR);
	}
	 
	// Write makefile
	fs::path makefilePath = projectRoot / "makefile";
	std::ofstream makefile(makefilePath);
	if (makefile.is_open()) {
		makefile << makefileTemplate.getFilledTemplate();
		makefile.close();
	}
	else {
		printf("[Error] Failed to write makefile.\n");
		return errorCode(ErrorCode::MAKEFILE_CREATION_ERROR);
	}

	// Create dependencies config
	fs::path dependenciesConfigPath = projectRoot / "dependencies.cfg";
	std::ofstream dependenciesConfig(dependenciesConfigPath);
	if (dependenciesConfig.is_open()) {
		dependenciesConfig << "# C-Build-System config v1\n";
		dependenciesConfig.close();
	}
	else {
		printf("[Error] Failed to write dependencies.cfg.\n");
		return errorCode(ErrorCode::DEPENDENCIES_CONFIG_CREATION_ERROR);
	}

	printf("Successfully create new project: %s\n", projectName.c_str());
	return errorCode(ErrorCode::SUCCESS);
}

#include <set>
#include <queue>

int pull() {

	fs::path config("dependencies.cfg");

	DependencyGraph dependencyGraph(config);

	// hardcoded for now
	std::string releaseLibsDir = "lib/release/x86_64";
	std::string debugLibsDir = "lib/debug/x86_64";

	std::stack<Dependency*> dependenciesToPull = dependencyGraph.getTopologicalOrder();
	std::set<Dependency> alreadyProcessed;
	std::queue<const Dependency*> linkOrder;
	while (!dependenciesToPull.empty()) {
		Dependency* dependencyToPull = dependenciesToPull.top();
		if (alreadyProcessed.find(*dependencyToPull) == alreadyProcessed.end()) {
			alreadyProcessed.insert(*dependencyToPull);
			linkOrder.push(dependencyToPull);
			std::cout << "Pulling " << dependencyToPull->repo << std::endl;
			// See and potentially fix how dependencies with conflicting namespaces are handled
			dependencyToPull->pullFiles("include/dependencies", releaseLibsDir + "/", debugLibsDir + "/");
		}
		dependenciesToPull.pop();
	}

	std::string libs;
	std::string libsDebug;
	while (!linkOrder.empty()) {
		const Dependency* dependency = linkOrder.front();
		linkOrder.pop();
		std::string makefileString = fs::path(dependency->releaseLibFilename).stem().string();
		std::string makefileStringDebug = fs::path(dependency->debugLibFilename).stem().string();
		libs += " -l \"" + makefileString + "\"";
		libsDebug += " -l \"" + makefileStringDebug + "\"";
	}

	std::cout << " -> " << libs << std::endl;
	std::cout << " -> " << libsDebug << std::endl;

	if (!make::replaceVarDeclaration("makefile", "LIBS_R", libs)) {
		std::cout << "Uh oh 1" << std::endl;
		return -1;
	}
	if (!make::replaceVarDeclaration("makefile", "LIBS_D", libsDebug)) {
		std::cout << "Uh oh 2" << std::endl;
		return -1;
	}

	return errorCode(ErrorCode::SUCCESS);
}

int package(const std::vector<std::string> arguments) {
	// not sure what I planned to do with the arguments

	fs::path packageDir("Package");
	if (fs::exists(packageDir)) {
		// need to check success
		fs::remove_all(packageDir);
	}
	fs::create_directory(packageDir);

	// Package dependencies.cfg
	fs::path config("dependencies.cfg");
	if (!fs::exists(config)) {
		// TODO handle error
	}

	if (!fs::copy_file(config, packageDir / config)) {
		// TODO handle error
	}


	// Package .lib files

	// hardcoded for now
	std::string releaseLibsDir = "lib/release/x86_64";
	std::string debugLibsDir = "lib/debug/x86_64";
	// incorrect assumption, libraries dont need to share name with project name
	fs::path releaseLib = fs::path(releaseLibsDir).make_preferred() / fs::path(fs::current_path().filename().string() + ".lib");
	fs::path debugLib = fs::path(debugLibsDir).make_preferred() / fs::path(fs::current_path().filename().string() + "-d.lib");

	fs::copy_file(releaseLib, packageDir / releaseLib.filename());
	fs::copy_file(debugLib, packageDir / debugLib.filename());


	// Package include.zip

	return errorCode(ErrorCode::UNIMPLEMENTED);
}

int main(const int argc, const char** argv) {
	//curlDriver();
	//localDependenciesTest();
	//remoteDependenciesTest();
	//remoteLocalMixTest();

	// 1-20-2024
	if (argc < 2) {
		return unexpectedArgCount();
	}
	else {
		std::vector<std::string> arguments;
		for (int i = 1; i < argc; i++) {
			arguments.push_back(std::string(argv[i]));
		}

		std::string action = arguments[0];
		if (action == "init") {
			return init(arguments);
		}
		if (action == "pull") {
			return pull();
		}
		if (action == "package") {
			return package(arguments);
		}
		else {
			return unresolvedAction(action);
		}
	}

	return 0;

	
	// pre 1-20-2024
	switch (argc) {
	//case 1: return newCurlTest();
	case 1 : return defaultBuild();
	default: return unexpectedArgCount();
	}

	return 0;
}