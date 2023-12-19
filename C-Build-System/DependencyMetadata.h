#pragma once
#include <string>

struct DependencyMetadata {
	std::string repo;
	std::string owner;
	std::string tag;

	bool operator<(const DependencyMetadata& other) const {
		if (repo != other.repo)  return repo < other.repo;
		if (owner != other.owner) return owner < other.owner;
		if (tag != other.tag) return tag < other.tag;
		return false;
	}
};