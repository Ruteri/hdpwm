#pragma once

#include <src/cli/fwd.h>
#include <src/cli/utils.h>

#include <src/crypto/utils.h>

#include <string>

class OutputHandler {
	virtual void m_draw(WINDOW *window) = 0;

  public:
	void draw();
	void draw(WINDOW *window);
};

class StringOutputHandler : public OutputHandler {
	Point origin;
	std::string output;

	void m_draw(WINDOW *window) override;

  public:
	StringOutputHandler(Point origin, std::string output) :
	    origin(origin), output(std::move(output)) {}
};

class SensitiveOutputHandler : public OutputHandler {
	Point origin;
	utils::sensitive_string sensitive_output;

	void m_draw(WINDOW *window) override;

  public:
	SensitiveOutputHandler(Point origin, utils::sensitive_string sensitive_output) :
	    origin(origin), sensitive_output(std::move(sensitive_output)) {}
};
