#include "Template.h"
#include <vector>
#include <utility>

Template::Template(const std::string& templateString) {
	this->templateString = templateString;
}

std::set<std::string> Template::getKeys() const {
	std::set<std::string> keys;

	bool keyFlag = false;
	std::string key;
	for (int i = 0; i < templateString.size(); i++) {
		char current = templateString[i];

		if (keyFlag and current != '}') {
			key += current;
		}
		
		if (keyFlag and i < templateString.size()-1 and current == '}' and templateString[i+1] == '}') {
			keys.insert(key);
			key.clear();
			keyFlag = false;
		}

		if (!keyFlag and i > 0 and current == '{' and templateString[i-1] == '{') {
			keyFlag = true;
		}
	}

	return keys;
}

bool Template::fillTemplate(const std::map<std::string, std::string>& inserts) {
	// ex. <debug, <245, 253>>
	std::vector<std::pair<std::string, std::pair<int, int>>> fillIns;

	bool keyFlag = false;
	std::string key;
	int start = -1;
	int end = -1;
	for (int i = 0; i < templateString.size(); i++) {
		char current = templateString[i];

		if (keyFlag and current != '}') {
			key += current;
		}

		if (keyFlag and i < templateString.size()-1 and current == '}' and templateString[i+1] == '}') {
			end = i+1;
			std::pair<std::string, std::pair<int, int>> fillIn;
			fillIn.first = key;
			fillIn.second.first = start;
			fillIn.second.second = end;
			fillIns.push_back(fillIn);
			key.clear();
			keyFlag = false;
		}

		if (!keyFlag and i > 0 and current == '{' and templateString[i-1] == '{') {
			keyFlag = true;
			start = i-1;
		}
	}

	// fill in the template
	filledOutTemplate = templateString;
	int offset = 0;
	for (auto fillIn : fillIns) {
		auto it = inserts.find(fillIn.first);
		if (it == inserts.end()) {
			// insert not provided for a fillIn
			return false;
		}
		std::string insertValue = it->second;
		std::string templateLeftSide = filledOutTemplate.substr(0, fillIn.second.first + offset);
		std::string templateRightSide = filledOutTemplate.substr(fillIn.second.second+1 + offset);
		filledOutTemplate = templateLeftSide + insertValue + templateRightSide;

		int removedCount = (fillIn.second.second - fillIn.second.first) + 1;
		int insertedCount = insertValue.size();
		offset += insertedCount - removedCount;
	}

	return true;
}

std::string Template::getFilledTemplate() const {
	return filledOutTemplate;
}