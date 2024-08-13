#pragma once

#include "CircularLogger.h"
#include "CaptureDevice.h"

#include <QSettings>

#define TO_INT(v) static_cast<int>(v)

class UdpRetranslatorApp
{
public:
	UdpRetranslatorApp();
	~UdpRetranslatorApp();

	bool init(int argc, char** argv);
	int run();

	static void startRetranslate();
	void waitQuitRequested();
	static void stopRetranslate();

private:
	bool loadNpcapDlls();

	void parseCmdLineArgs();

	void printHelp();
	bool getCaptureDevices();
	bool printCaptureDevices();
	bool testCaptureDevice();
	bool readCfgFile(const QString& cfgFileName);
	bool saveCfgFileName(const QString& cfgFileName);

	bool runService();

private:
	inline static const QString ARG_HELP = QStringLiteral("-h");
	inline static const QString ARG_DEV_LIST = QStringLiteral("-devlist");
	inline static const QString ARG_TEST_CAP = QStringLiteral("-testcap");
	inline static const QString ARG_CFG = QStringLiteral("-cfg");

	int m_argc = 0;
	char** m_argv = nullptr;

	QString m_appPathFile;
	std::map<QString, QString> m_cmdLineArgs;

	std::vector<CaptureDevice> m_captureDevices;

	std::vector<RetranslateCfg> m_retranslateCfgs;

	inline static bool m_instanceCreated = false;
	inline static std::mutex m_waitQuitMutex;
	inline static std::condition_variable m_waitQuit;
	inline static bool m_quitRequested = false;

	inline static const QString CFG_FILE_NAME = "CfgFileName";
};

extern CircularLoggerShared logger;
extern UdpRetranslatorApp app;
extern QSettings settings;

VOID serviceMain(DWORD argc, LPTSTR* argv);
VOID serviceCtrlHandler(DWORD CtrlCode);

BOOL WINAPI consoleCtrlHandler(_In_ DWORD dwCtrlType);
