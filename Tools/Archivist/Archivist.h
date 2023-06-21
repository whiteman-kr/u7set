#pragma once

#include "../../ServiceLib/CommandLineParser.h"

class Archivist
{
public:
	Archivist(int argc, char* argv[]);

	bool start();

	virtual void copyArchive() = 0;

private:
	CommandLineParser m_archivierCmdParser;

	QString m_sourceDir;
	QString m_beginTime;
	QString m_endTime;
	QString m_destFile;
	int m_partSize = 0;
};
