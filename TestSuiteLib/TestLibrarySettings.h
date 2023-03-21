#pragma once

#include "../CommonLib/HostAddressPort.h"

class TestLibrarySettings
{
public:
	TestLibrarySettings();

	void restoreFromRegistry();
	bool restoreFromFile(const QString& fileName, QString* errorMsg);

	void saveToRegistry();

	static bool createTemplateConfigurationFile(const QString& fileName);

	QString instanceStrId() const;
	void setInstanceStrId(const QString& value);

	void setConfiguratorAddress1(const QString& address, int port);
	HostAddressPort configuratorAddress1() const;

	void setConfiguratorAddress2(const QString& address, int port);
	HostAddressPort configuratorAddress2() const;

	bool loadScriptsFromPath() const;
	void setLoadScriptsFromPath(bool value);

	QString scriptsPath() const;
	void setScriptsPath(const QString& path);

private:
	bool getArgumentFromXml(QDomElement& docElem, QString name, QString* result);
	bool getArgumentFromXml(QDomElement& docElem, QString name, int* result);

private:
	QString m_instanceStrId;
	QString m_configuratorIpAddress1;
	int m_configuratorPort1 = 0;
	QString m_configuratorIpAddress2;
	int m_configuratorPort2 = 0;

	bool m_loadScriptsFromPath = false;
	QString m_scriptsPath;
};

