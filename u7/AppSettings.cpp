#include "AppSettings.h"
#include "./ProjectsTabPage/ProjectBackup.h"

#include "../Tools/qtkeychain/keychain.h"


//
//	DatabaseConnectionParam
//
QString DatabaseConnectionParam::address() const
{
	return QString(m_address);
}

void DatabaseConnectionParam::setAddress(QString str)
{
	memset(m_address, 0, sizeof(DatabaseConnectionParam::m_address));
	if (str.size() >= sizeof(DatabaseConnectionParam::m_address) / sizeof(DatabaseConnectionParam::m_address[0]) - 1)
	{
		return;
	}

	for (int i = 0; i < str.size(); i++)
	{
		m_address[i] = str.constData()[i];
	}
}

int DatabaseConnectionParam::port() const
{
	return m_port;
}

void DatabaseConnectionParam::setPort(int port)
{
	m_port = port;
}

QString DatabaseConnectionParam::login() const
{
	return QString(m_login);
}

void DatabaseConnectionParam::setLogin(QString str)
{
	memset(m_login, 0, sizeof(DatabaseConnectionParam::m_login));
	if (str.size() >= sizeof(DatabaseConnectionParam::m_login) / sizeof(DatabaseConnectionParam::m_login[0]) - 1)
	{
		return;
	}

	for (int i = 0; i < str.size(); i++)
	{
		m_login[i] = str.constData()[i];
	}
}

QString DatabaseConnectionParam::password() const
{
	return QString(m_password);
}

void DatabaseConnectionParam::setPassword(QString str)
{
	memset(m_password, 0, sizeof(DatabaseConnectionParam::m_password));
	if (str.size() >= sizeof(DatabaseConnectionParam::m_password) / sizeof(DatabaseConnectionParam::m_password[0]) - 1)
	{
		return;
	}

	for (int i = 0; i < str.size(); i++)
	{
		m_password[i] = str.constData()[i];
	}
}

bool AppSettings::m_expertModeCached = false;


AppSettings::AppSettings() :
#ifdef Q_OS_LINUX
	m_configuratorSerialPort("ttyS0"),
#endif
#ifdef Q_OS_WIN32
	m_configuratorSerialPort("\\\\.\\COM3"),
#endif
	m_buildOutputPath(QDir().toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))),
	m_expertMode(false)
{
}

void AppSettings::load()
{
	QSettings fallbackSettings{};
	QByteArray raw;

	// Database connection setting are stored in secure storage
	//
	{
		QKeychain::ReadPasswordJob readJob(QLatin1String("u7keychain18"));
		readJob.setInsecureFallback(true);
		readJob.setSettings(&fallbackSettings);
		readJob.setKey("f1646f45-238a-45ec-ad0c-0d0960067b96");

		QEventLoop loop;
		readJob.connect(&readJob, &QKeychain::ReadPasswordJob::finished, &loop, &QEventLoop::quit);

		readJob.start();
		loop.exec();

		if (readJob.error() != QKeychain::Error::NoError)
		{
			qDebug() << "Restoring keychain failed: " << readJob.errorString();

			// One more fallback, the final one
			//
			if (fallbackSettings.contains(readJob.key()) == true)
			{
				raw = fallbackSettings.value(readJob.key()).toByteArray();
			}
		}
		else
		{
			raw = readJob.binaryData();
		}
	}

	if (raw.size() == sizeof(m_databaseConnection))
	{
		std::memcpy(&m_databaseConnection, raw.constData(), sizeof(m_databaseConnection));
	}
	else
	{
		qDebug() << "Default params will be used.";

		// Set default params
		//
		m_databaseConnection.setAddress("127.0.0.1");
		m_databaseConnection.setPort(5432);
		m_databaseConnection.setLogin("u7");
		m_databaseConnection.setPassword("P2ssw0rd");
	}

	// Other settings
	//
	QSettings s;

	m_buildOutputPath = s.value("m_buildOutputPath", m_buildOutputPath).toString();
	m_expertMode = s.value("Main/m_expertMode", false).toBool();

	m_pgDumpCommand = s.value("m_pgDumpCommand", "AUTODTECT").toString();
	m_psqlCommand = s.value("m_psqlCommand", "AUTODTECT").toString();

	if (m_pgDumpCommand == "AUTODTECT")
	{
		m_pgDumpCommand = ProjectBackup::autoDetectExecutable("pg_dump");
	}

	if (m_psqlCommand == "AUTODTECT")
	{
		m_psqlCommand = ProjectBackup::autoDetectExecutable("psql");
	}

	m_expertModeCached = m_expertMode;

	m_configuratorSerialPort = s.value("m_configuratorSerialPort", m_configuratorSerialPort).toString();
	m_configuratorShowDebugInfo = s.value("m_configuratorShowDebugInfo", m_configuratorShowDebugInfo).toBool();
	m_configuratorVerify = s.value("m_configuratorVerify", m_configuratorVerify).toBool();

	return;
}

void AppSettings::save() const
{
	QSettings fallbackSettings{};

	// Database connection setting are sored in secure storage
	//
	{
		{
			// Somehow credentials returns to first written state after log out/log in
			//
			QKeychain::DeletePasswordJob deleteJob(QLatin1String("u7keychain18"));
			deleteJob.setInsecureFallback(true);
			deleteJob.setSettings(&fallbackSettings);
			deleteJob.setKey("f1646f45-238a-45ec-ad0c-0d0960067b96");

			// Blocking job
			//
			QEventLoop loop;
			deleteJob.connect(&deleteJob, &QKeychain::DeletePasswordJob::finished, &loop, &QEventLoop::quit);

			deleteJob.start();
			loop.exec();

			if (deleteJob.error() != QKeychain::Error::NoError)
			{
				qDebug() << "Deleting keychain failed: " << deleteJob.errorString();
			}
		}

		QKeychain::WritePasswordJob writeJob("u7keychain18");
		writeJob.setInsecureFallback(true);
		writeJob.setSettings(&fallbackSettings);
		writeJob.setKey("f1646f45-238a-45ec-ad0c-0d0960067b96");

		QByteArray ba = QByteArray::fromRawData(reinterpret_cast<const char*>(&m_databaseConnection), sizeof(m_databaseConnection));
		writeJob.setBinaryData(ba);

		// Blocking job
		//
		QEventLoop loop;
		writeJob.connect(&writeJob, &QKeychain::WritePasswordJob::finished, &loop, &QEventLoop::quit);

		writeJob.start();
		loop.exec();

		if (writeJob.error() != QKeychain::Error::NoError)
		{
			qDebug() << "Storing keychain failed: " << writeJob.errorString();
			qDebug() << "Suggestion: Install libsecret, gnome-keyring";

			// One more fallback, the final one
			//
			fallbackSettings.setValue(writeJob.key(), ba);
		}
	}

	// Save other settings
	//
	QSettings s;

	s.setValue("m_buildOutputPath", m_buildOutputPath);
	s.setValue("Main/m_expertMode", m_expertMode);

	s.setValue("m_pgDumpCommand", m_pgDumpCommand);
	s.setValue("m_psqlCommand", m_psqlCommand);

	m_expertModeCached = m_expertMode;

	s.setValue("m_configuratorSerialPort", m_configuratorSerialPort);
	s.setValue("m_configuratorShowDebugInfo", m_configuratorShowDebugInfo);
	s.setValue("m_configuratorVerify", m_configuratorVerify);

	return;
}

QString AppSettings::serverHost() const
{
	return m_databaseConnection.address();
}
void AppSettings::setServerHost(const QString& value)
{
	m_databaseConnection.setAddress(value);
}

int AppSettings::serverPort() const
{
	return m_databaseConnection.port();
}

void AppSettings::setServerPort(int value)
{
	m_databaseConnection.setPort(value);
}

QString AppSettings::serverUsername() const
{
	return m_databaseConnection.login();
}

void AppSettings::setServerUsername(const QString& value)
{
	m_databaseConnection.setLogin(value);
}

QString AppSettings::serverPassword() const
{
	return m_databaseConnection.password();
}

void AppSettings::setServerPassword(const QString& value)
{
	m_databaseConnection.setPassword(value);
}

const QString& AppSettings::buildOutputPath() const
{
	return m_buildOutputPath;
}

void AppSettings::setBuildOutputPath(const QString& value)
{
	m_buildOutputPath = value;
}

bool AppSettings::isExpertModeCached()
{
	return m_expertModeCached;
}

bool AppSettings::isExpertMode() const
{
	return m_expertMode;
}

void AppSettings::setExpertMode(bool value)
{
	m_expertMode = value;
}

QString AppSettings::pgDumpCommand() const
{
	return m_pgDumpCommand;
}

void AppSettings::setPgDumpCommand(const QString& value)
{
	m_pgDumpCommand = value;
}

QString AppSettings::psqlCommand() const
{
	return m_psqlCommand;
}

void AppSettings::setPsqlCommand(const QString& value)
{
	m_psqlCommand = value;
}

QString AppSettings::configuratorSerialPort() const
{
	return m_configuratorSerialPort;
}

void AppSettings::setConfiguratorSerialPort(const QString& value)
{
	m_configuratorSerialPort = value;
}

bool AppSettings::configuratorShowDebugInfo() const
{
	return m_configuratorShowDebugInfo;
}

void AppSettings::setConfiguratorShowDebugInfo(bool value)
{
	m_configuratorShowDebugInfo = value;
}

bool AppSettings::configuratorVerify() const
{
	return m_configuratorVerify;
}

void AppSettings::setConfiguratorVerify(bool value)
{
	m_configuratorVerify = value;
}
