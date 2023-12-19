#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include "DependencyMetadata.h"
#include "ConfigFileHelper.h"
#include "Curl.h"
#include "MinizWrapper.h"

namespace fs = std::filesystem;

// C++ is stupid, need anonymous namespace or static member will cause linker errors
namespace {

class Dependency {
public:
	static std::string localReposDir;
	std::string repo; // "." means the current project
	std::string owner; // "local" means dependency is not to be retrieved remotely; it is the user's responsibility
	std::string tag; // ex: v1.0
	std::vector<Dependency> directDependencies;

	Dependency() {}
	Dependency(const std::string& repo, const std::string& owner, const std::string& tag = "") {
		this->repo = repo;
		this->owner = owner;
		this->tag = tag;
	}
	~Dependency() {}

	bool fetchDependencyConfigs() {
		if (repo.empty() or owner.empty()) return false;

		std::cout << "fetching repo " << repo << std::endl;

		//if (isLocal()) {
		//	std::vector<std::string> configLines = helpers::readDependencyConfig(fs::path(localReposDir)/owner/repo/tag/"dependencies.cfg");
		//	std::set<DependencyMetadata> directDependenciesMetadata = helpers::parseDependencyConfig(configLines);
		//	setDirectDependencies(directDependenciesMetadata);
		//}

		std::vector<std::string> configLines;

		if (isCached()) {
			configLines = helpers::readDependencyConfig(fs::path(localReposDir)/owner/repo/tag/"dependencies.cfg");
		}
		else { // Otherwise retrieve from S3
			configLines = fetchRemoteDependencyConfig();
		}

		if (!configLines.empty()) {
			std::set<DependencyMetadata> directDependenciesMetadata = helpers::parseDependencyConfig(configLines);
			setDirectDependencies(directDependenciesMetadata);
		}

		return true;
	}

	// Not used yet. May not need it.
	void fetchDependencies(std::set<Dependency>* cachedDependencies) const {
		for (const Dependency dependency : directDependencies) {
			if (cachedDependencies->find(dependency) != cachedDependencies->cend()) {
				dependency.fetchDependencies(cachedDependencies);
				cachedDependencies->insert(dependency);
			}
		}

		// actually download and cache dependencies
	}

	bool pullFiles(const std::string& includesDest, const std::string& releaseLibsDir, const std::string& debugLibsDir) const {
		std::string baseUrl = "https://github-repos-371306945901.s3.amazonaws.com";
		baseUrl += "/" + owner;
		baseUrl += "/" + repo;
		baseUrl += "/" + tag + "/";

		std::string fileListUrl = baseUrl + "file_list.txt";

		fs::path outputLocation(localReposDir);
		outputLocation /= owner;
		outputLocation /= repo;
		outputLocation /= tag;

		cbs::CurlCommand curlCommand;
		cbs::CurlFileArguments curlFileArgs;
		curlFileArgs.fileUrl = fileListUrl;
		curlFileArgs.outputFilepath = outputLocation/"file_list.txt";
		curlCommand.fileArguments.push_back(curlFileArgs);

		if (!fs::exists(curlCommand.fileArguments[0].outputFilepath)) {
			if (!cbs::curl(curlCommand)) {
				// TODO handle left over files gracefully or cause error
				return false;
			}
		}

		std::vector<std::string> fileList = helpers::readLines(curlCommand.fileArguments[0].outputFilepath);
		std::string debugLibFilename = getDebugLibFilename(fileList);
		std::string releaseLibFilename = getReleaseLibFilename(fileList);

		for(std::string filename : fileList) {
			curlCommand.fileArguments[0].fileUrl = baseUrl + filename;
			curlCommand.fileArguments[0].outputFilepath = outputLocation/filename;

			if (!fs::exists(curlCommand.fileArguments[0].outputFilepath)) {
				if (!cbs::curl(curlCommand)) {
					// TODO handle left over files gracefully or cause error
					return false;
				}
			}

			if (filename == "include.zip") {
				fs::create_directories(includesDest);
				fs::path filepath(localReposDir);
				filepath /= owner;
				filepath /= repo;
				filepath /= tag;
				filepath /= filename;
				if (zip::unzip(filepath.string().c_str(), includesDest.c_str()) != 0) {
					// TODO handle left over files gracefully or cause error
					return false;
				}
			}
			if (filename == debugLibFilename) { // its a debug library
				fs::create_directories(debugLibsDir);
				if (!fs::exists(debugLibsDir + filename) && !fs::copy_file(curlCommand.fileArguments[0].outputFilepath, debugLibsDir + filename)) {
					// TODO handle left over files gracefully or cause error
					return false;
				}
			}
			if (filename == releaseLibFilename) { // its a release library
				fs::create_directories(releaseLibsDir);
				if (!fs::exists(releaseLibsDir + filename) && !fs::copy_file(curlCommand.fileArguments[0].outputFilepath, releaseLibsDir + filename)) {
					// TODO handle left over files gracefully or cause error
					return false;
				}
			}
		}

		return true;
	}

	void setDirectDependencies(const std::set<DependencyMetadata>& dependencyMetadatas) {
		for (const auto dependencyMetadata : dependencyMetadatas) {
			directDependencies.emplace_back(dependencyMetadata.repo, dependencyMetadata.owner, dependencyMetadata.tag);
		}
	}

	bool operator<(const Dependency& other) const  {
		if (repo  != other.repo)  return repo  < other.repo;
		if (owner != other.owner) return owner < other.owner;
		if (tag   != other.tag)   return tag < other.tag;
		return false;
	}

private:
	bool isLocal() const {
		return owner == "local";
	}

	bool isCached() const {
		return fs::exists(fs::path(localReposDir)/owner/repo/tag/"dependencies.cfg");
	}

	std::string getDebugLibFilename(const std::vector<std::string>& fileList) const {		
		std::vector<std::string> libs;
		for (auto file : fileList) {
			if (file.find(".lib") != std::string::npos) {
				libs.push_back(file);
			}
		}

		if (libs.size() == 1) return libs.front();
		else if (libs.size() == 2) {
			for (auto lib : libs) {
				if (lib.find("-d.lib") != std::string::npos) {
					return lib;
				}
			}
		}
		else {
			// TODO cause error here
			return std::string();
		}
	}

	std::string getReleaseLibFilename(const std::vector<std::string>& fileList) const {
		std::vector<std::string> libs;
		for (auto file : fileList) {
			if (file.find(".lib") != std::string::npos) {
				libs.push_back(file);
			}
		}

		if (libs.size() == 1) return libs.front();
		else if (libs.size() == 2) {
			for (auto lib : libs) {
				if (lib.find("-d.lib") == std::string::npos) {
					return lib;
				}
			}
		}
		else {
			// TODO cause error here
			return std::string();
		}
	}

	// performs system call to curl to fetch config file from S3
	std::vector<std::string> fetchRemoteDependencyConfig() {
		std::vector<std::string> contents;

		std::string dependencyConfigUrl = "https://github-repos-371306945901.s3.amazonaws.com";
		dependencyConfigUrl += "/" + owner;
		dependencyConfigUrl += "/" + repo;
		dependencyConfigUrl += "/" + tag;
		dependencyConfigUrl += "/dependencies.cfg";

		fs::path outputLocation(localReposDir);
		outputLocation /= owner;
		outputLocation /= repo;
		outputLocation /= tag;

		if (!fs::create_directories(outputLocation)) {
			// TODO cause error here
			return contents;
		}

		outputLocation /= "dependencies.cfg";
		
		cbs::CurlCommand curlCommand;
		cbs::CurlFileArguments curlFileArgs;
		curlFileArgs.fileUrl = dependencyConfigUrl;
		curlFileArgs.outputFilepath = outputLocation;
		curlCommand.fileArguments.push_back(curlFileArgs);

		if (!cbs::curl(curlCommand)) {
			// TODO cause error here
			return contents;
		}

		contents = helpers::readDependencyConfig(outputLocation);

		return contents;
	}
};

std::string Dependency::localReposDir = "PackageCache";
}