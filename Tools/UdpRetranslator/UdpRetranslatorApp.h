#pragma once

#include "CircularLogger.h"
#include "CaptureDevice.h"

#define TO_INT(v) static_cast<int>(v)

class UdpRetranslatorApp
{
public:
	UdpRetranslatorApp(int argc, char** argv);

	int run();

private:
	bool loadNpcapDlls();

	void parseCmdLineArgs();

	void printHelp();
	bool getCaptureDevices();
	bool printCaptureDevices();
	bool testCaptureDevice();

	bool runService();

private:
	inline static const QString ARG_HELP = QStringLiteral("-h");
	inline static const QString ARG_DEV_LIST = QStringLiteral("-devlist");
	inline static const QString ARG_TEST_CAP = QStringLiteral("-testcap");

	int m_argc = 0;
	char** m_argv = nullptr;

	CircularLoggerShared m_log;
	QString m_appPathFile;
	std::map<QString, QString> m_cmdLineArgs;

	std::vector<CaptureDevice> m_captureDevices;
};
