/*

Copyright (C) 2019 Mateusz Morusiewicz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

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
