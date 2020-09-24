#pragma once

class ILogFile
{
public:
	virtual bool writeAlert(const QString& text) = 0;
	virtual bool writeError(const QString& text) = 0;
	virtual bool writeWarning(const QString& text) = 0;
	virtual bool writeMessage(const QString& text) = 0;
	virtual bool writeText(const QString& text) = 0;
};


