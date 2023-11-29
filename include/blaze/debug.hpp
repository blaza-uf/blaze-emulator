#pragma once

#include <string>

namespace Blaze {
	void clear();
	void print(const std::string& subsystem, const std::string& message);
	void printLine(const std::string& subsystem, const std::string& message);
};
