#pragma once

#include "CircularLogger.h"

class UdpRetranslatorApp
{
public:
	UdpRetranslatorApp(int argc, char** argv);

	int run();

private:
	void parseCmdLineArgs();

	void printHelp();

private:
	int m_argc = 0;
	char** m_argv = nullptr;

	QString m_appPathFile;
	std::map<QString, QString> m_cmdLineArgs;
};
