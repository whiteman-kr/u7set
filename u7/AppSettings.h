#pragma once


struct DatabaseConnectionParam
{
	QChar m_address[256];
	qint32 m_port;
	QChar m_login[256];
	QChar m_password[256];

	QString address() const;
	void setAddress(QString str);

	int port() const;
	void setPort(int port);

	QString login() const;
	void setLogin(QString str);

	QString password() const;
	void setPassword(QString str);
};


class AppSettings
{
public:
	AppSettings();

public:
	void load();
	void save() const;

public:
	QString serverHost() const;
	void setServerHost(const QString& value);

	int serverPort() const;
	void setServerPort(int value);

	QString serverUsername() const;
	void setServerUsername(const QString& value);

	QString serverPassword() const;
	void setServerPassword(const QString& value);

	const QString& buildOutputPath() const;
	void setBuildOutputPath(const QString& value);

	bool isExpertMode() const;
	void setExpertMode(bool value);

	QString pgDumpCommand() const;
	void setPgDumpCommand(const QString& value);

	QString psqlCommand() const;
	void setPsqlCommand(const QString& value);

	QString configuratorSerialPort() const;
	void setConfiguratorSerialPort(const QString& value);

	bool configuratorShowDebugInfo() const;
	void setConfiguratorShowDebugInfo(bool value);

	bool configuratorVerify() const;
	void setConfiguratorVerify(bool value);

private:
	DatabaseConnectionParam m_databaseConnection{};
	QString m_buildOutputPath;

	bool m_expertMode = false;
	QString m_pgDumpCommand;
	QString m_psqlCommand;

	// Configurator properties
	//
	QString m_configuratorSerialPort;
	bool m_configuratorShowDebugInfo = false;
	bool m_configuratorVerify = true;
};

extern AppSettings theAppSettings;