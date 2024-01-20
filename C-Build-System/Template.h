#pragma once
#include <string>
#include <map>
#include <set>

class Template
{
private:
	std::string templateString;
	std::string filledOutTemplate;

public:
	Template(const std::string& templateString);
	~Template() {}

	std::set<std::string> getKeys() const;
	bool fillTemplate(const std::map<std::string, std::string>& inserts);
	std::string getFilledTemplate() const;
};

