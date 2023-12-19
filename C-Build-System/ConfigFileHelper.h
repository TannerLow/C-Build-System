#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>
#include "DependencyMetadata.h"

namespace helpers {

	namespace fs = std::filesystem;

	std::vector<std::string> splitString(const std::string& str);

	inline std::vector<std::string> readDependencyConfig(std::ifstream& file) {
		std::vector<std::string> configLines;

		if (file.is_open()) {
			while (file.good()) {
				std::string line;
				std::getline(file, line);
				if (!line.empty() and line[0] != '#') {
					configLines.push_back(line);
				}
			}

			file.close();
		}
		else {
			std::cout << "No open file handle to read from" << std::endl;
		}

		return configLines;
	}

	inline std::vector<std::string> readDependencyConfig(const fs::path& path) {
		if (!fs::exists(path)) {
			std::cout << "Couldn't find file" << std::endl;
			return std::vector<std::string>();
		}
		else {
			std::cout << "Reading dependency config:" << path << std::endl;
			std::ifstream file;
			file.open(path);
			//std::cout << file.rdbuf() << std::endl;
			return readDependencyConfig(file);
		}
	}

	inline std::set<DependencyMetadata> parseDependencyConfig(const std::vector<std::string>& configLines) {
		std::set<DependencyMetadata> dependencies;

		for (const auto line : configLines) {
			std::vector<std::string> tokens = splitString(line);
			if (tokens.size() > 1) {
				DependencyMetadata dependency;
				dependency.repo = tokens[0];
				dependency.owner = tokens[1];
				if (tokens.size() > 2) {
					dependency.tag = tokens[2];
				}
				dependencies.insert(dependency);
			}
		}

		return dependencies;
	}

	// splits string by space (' ')
	// 
	// Q: what about multiple spaces or other whitespace?
	inline std::vector<std::string> splitString(const std::string& str) {
		std::vector<std::string> tokens;
		std::stringstream ss;
		ss << str;

		std::string token;
		while (ss >> token) {
			tokens.push_back(token);
		}

		return tokens;
	}

	inline std::vector<std::string> readLines(const fs::path& path) {
		std::vector<std::string> lines;

		std::ifstream file(path);
		std::string line;
		while (std::getline(file, line)) {
			lines.push_back(line);
		}
		file.close();

		return lines;
	}
}