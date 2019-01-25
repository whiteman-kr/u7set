#ifndef UALTESTER_H
#define UALTESTER_H

#include <QObject>

#include "../../lib/HostAddressPort.h"

class UalTester : public QObject
{
	Q_OBJECT

public:

	UalTester(int& argc, char** argv);
	~UalTester();

protected:

	static const char* const SETTING_CFG_SERVICE_IP1;
	static const char* const SETTING_CFG_SERVICE_IP2;
	static const char* const SETTING_EQUIPMENT_ID;
	static const char* const SETTING_TEST_FILE_NAME;

private:

	QString			m_cfgServiceIP1Str;
	QString			m_cfgServiceIP2Str;
	QString			m_equipmentID;
	QString			m_testFileName;

	HostAddressPort m_cfgSocketAddress1;
	HostAddressPort m_cfgSocketAddress2;

	void			getCmdLineParams(int& argc, char** argv);
	bool			cmdLineParamsIsValid();

public:

	bool			start();

signals:

public slots:

};

#endif // UALTESTER_H
