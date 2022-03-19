#pragma once
#include <vector>
#include <string>

class Command {
public:
	static std::vector<std::string> SplitCommand(const std::string& parent);
	static void CustomCmd(std::string cmd, std::string group);
};