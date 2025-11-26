#pragma once

#include <string_view>

//
// TODO: Move this interface out of this lib? to CommonStdLib?
//

class ILoggerStd
{
public:
	virtual ~ILoggerStd() = default;

	virtual void writeAlert(std::string_view message) = 0;
	virtual void writeError(std::string_view message) = 0;
	virtual void writeWarning(std::string_view message) = 0;
	virtual void writeMessage(std::string_view message) = 0;
};