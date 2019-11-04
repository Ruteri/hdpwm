#pragma once

#include <src/cli/utils.h>

#include <string>

class OutputHandler {
	Point origin;
	std::string output;

public:
	OutputHandler(Point origin, std::string output): origin(origin), output(std::move(output)) {}

	void draw();
};

