#pragma once

#include "../CommonLib/HostAddressPort.h"

namespace TestSuite
{
	class TestSuiteSettings
	{
	public:
		TestSuiteSettings();

		void restoreFromRegistry();
		bool restoreFromFile(const QString& fileName, QString* errorMsg);

		void saveToRegistry();

		static bool createTemplateSettingsFile(const QString& fileName);

		QString instanceStrId() const;
		void setInstanceStrId(const QString& value);

		void setConfiguratorAddress1(const HostAddressPort& address);
		HostAddressPort configuratorAddress1() const;

		void setConfiguratorAddress2(const HostAddressPort& address);
		HostAddressPort configuratorAddress2() const;

//		bool loadScriptsFromPath() const;
//		void setLoadScriptsFromPath(bool value);

//		QString scriptsPath() const;
//		void setScriptsPath(const QString& path);

	private:
		bool getArgumentFromXml(QDomElement& docElem, QString name, QString* result);
		bool getArgumentFromXml(QDomElement& docElem, QString name, int* result);

	private:
		QString m_instanceStrId;

		HostAddressPort m_cfgServiceAddress1;
		HostAddressPort m_cfgServiceAddress2;

//		bool m_loadScriptsFromPath = false;
//		QString m_scriptsPath;
	};
}
