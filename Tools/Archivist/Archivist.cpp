#include "Archivist.h"

Archivist::Archivist(int argc, char* argv[]) :
	m_archivierCmdParser("Radiy", "Archivist", argc, argv)
{
	m_archivierCmdParser.addSimpleNoWritableCmdLineArg("copyfile",
													 "Copy file archive");

	m_archivierCmdParser.addValueNoWritebleCmdLineArg("src",
													"archive source directory",
													"d:\\archive\\project\\ARCH_SERVICE_ID");

	m_archivierCmdParser.addValueNoWritebleCmdLineArg("begin",
													"Start time of copy",
													"00:01:23 21.08.2023");

	m_archivierCmdParser.addValueNoWritebleCmdLineArg("end",
													"End time of copy",
													"00:01:00 23.08.2023");

	m_archivierCmdParser.addValueNoWritebleCmdLineArg("dest",
													"Name of archive file copy",
													"d:\\archive_copy_file");

	m_archivierCmdParser.addValueNoWritebleCmdLineArg("partsize",
													"Archive copy parts size, GB",
													"4");
}

bool Archivist::start()
{
	std::cout << C_STR(m_archivierCmdParser.helpText());

	m_archivierCmdParser.parseAndApplyCmdLineArgs();

	m_sourceDir = m_archivierCmdParser.getCmdLineArgValue("src");
	m_beginTime = m_archivierCmdParser.getCmdLineArgValue("begin");
	m_endTime = m_archivierCmdParser.getCmdLineArgValue("end");
	m_destFile = m_archivierCmdParser.getCmdLineArgValue("dest");

	bool ok = false;

	m_partSize = m_archivierCmdParser.getCmdLineArgValue("partsize").toInt(&ok);

	if (ok == false)
	{
		qDebug() << "Error partsize!";
		return false;
	}

	copyArchive();

//	if (archivierCmdParser.cmdLineArgIsSet("copyfile") == true)
//	{
//		qDebug() << "is set";
//	}
//	else
//	{
//		qDebug() << "is NOT set";
//	}


	return true;
}

