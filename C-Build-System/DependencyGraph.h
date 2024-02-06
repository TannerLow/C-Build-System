#pragma once
#include <string>
#include <set>
#include <stack>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Dependency.h"
#include "ConfigFileHelper.h"

namespace fs = std::filesystem;

class DependencyGraph {
private:
	Dependency root;

public:
	DependencyGraph(const fs::path& dependencyConfig) {
		std::vector<std::string> configLines = helpers::readDependencyConfig(dependencyConfig);
		root.setDirectDependencies(helpers::parseDependencyConfig(configLines));
		root.repo = ".";
		root.owner = "root";
		build(root);
	}

	~DependencyGraph() {}

	void build(Dependency& currentDependency) {
		// TODO add caching to avoid fetching same dependency subgraph twice
		for (Dependency& dependency : currentDependency.directDependencies) {
			if (!dependency.fetchDependencyConfigs()) {
				std::cout << "[ERROR] Failed to fetch dependencies for: Repo -> " + dependency.repo << std::endl;
			} 
			build(dependency);
		}
	}

	std::stack<Dependency*> getTopologicalOrder() {
		std::stack<Dependency*> dependencies;
		std::stack<Dependency*> dependenciesToVisit;
		dependenciesToVisit.push(&root);

		while (!dependenciesToVisit.empty()) {
			Dependency* currentDependency = dependenciesToVisit.top();
			dependenciesToVisit.pop();
			std::cout << "Repo: " << currentDependency->repo << "\tOwner: " << currentDependency->owner << "\tTag: " << currentDependency->tag << std::endl;
			for (auto& dependency : currentDependency->directDependencies) {
				std::cout << currentDependency->repo << " -> " << dependency.repo << std::endl;
				dependenciesToVisit.push(&dependency);
				dependencies.push(&dependency);
			}
		}

		return dependencies;
	}

	void print() const {
		std::stack<Dependency> dependenciesToVisit;
		dependenciesToVisit.push(root);

		while (!dependenciesToVisit.empty()) {
			const Dependency currentDependency = dependenciesToVisit.top();
			dependenciesToVisit.pop();
			std::cout << "Repo: " << currentDependency.repo << "\tOwner: " << currentDependency.owner << std::endl;
			for (auto dep : currentDependency.directDependencies) {
				std::cout << currentDependency.repo << " -> " << dep.repo << std::endl;
			}
			for (const auto dependency : currentDependency.directDependencies) {
				dependenciesToVisit.push(dependency);
			}
		}
	}
};

