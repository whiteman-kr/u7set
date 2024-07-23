#include "UdpRetranslatorApp.h"

UdpRetranslatorApp::UdpRetranslatorApp(int argc, char** argv) :
	m_argc(argc),
	m_argv(argv)
{
}

int UdpRetranslatorApp::run()
{
	parseCmdLineArgs();

	std::shared_ptr<CircularLogger> log = std::make_shared<CircularLogger>();
	circularLoggerInit(log, m_appPathFile, "udprtr", "", 10, 10);

	if (m_cmdLineArgs.contains("-h"))
	{
		printHelp();
	}

	//

	circularLoggerShutdown(log);

	return 0;
}

void UdpRetranslatorApp::parseCmdLineArgs()
{
	m_appPathFile = m_argv[0];

	m_cmdLineArgs.clear();

	for(int i = 1; i < m_argc; i++)
	{
		QString arg(m_argv[i]);

		QStringList l = arg.split("=", Qt::SkipEmptyParts);

		if (l.size() == 1)
		{
			m_cmdLineArgs.emplace(l[0].trimmed().toLower(), QString());
		}
		else
		{
			m_cmdLineArgs.emplace(l[0].trimmed().toLower(), l[1].trimmed());
		}
	}
}

void UdpRetranslatorApp::printHelp()
{
	qDebug() << "\n";
	qDebug() << "\nUse UdpRtr.exe [options]\n";
	qDebug() << "where options is:\n";
	qDebug() << "-h\t\tprint this help";
	qDebug() << "-e\t\trun UDP retranslator as console application\n";
	qDebug() << "\n";
}
