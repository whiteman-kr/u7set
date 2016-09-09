#pragma once

class Settings
{
public:
	Settings(void);
	virtual ~Settings(void);

public:
	void save() const;
	void load();

	QString serialPort() const;
	void setSerialPort(const QString& value);

	bool showDebugInfo() const;
	void setShowDebugInfo(bool value);

	bool verify() const;
	void setVerify(bool value);

	bool expertMode() const;
	void setExpertMode(bool value);

	QString server() const;
	void setServer(const QString& value);

	QString serverUsername() const;
	void setServerUsername(const QString& value);

	QString serverPassword() const;
	void setServerPassword(const QString& value);

private:
	QString m_serialPort;
	bool m_showDebugInfo;
	bool m_verify;
	bool m_expertMode;

	QString m_server;
	QString m_serverUsername;
	QString m_serverPassword;
};

