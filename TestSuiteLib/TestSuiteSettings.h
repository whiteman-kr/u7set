#pragma once

#include <QDomDocument>
#include "../CommonLib/HostAddressPort.h"

class QSettings;

namespace TestSuite
{
	class TestSuiteSettings
	{
	public:
		TestSuiteSettings();

		void restoreFromRegistry(const QSettings& s);
		bool restoreFromFile(const QString& fileName, QString* errorMsg);

		void saveToRegistry(QSettings& s) const;

		static bool createTemplateSettingsFile(const QString& fileName);

		QString instanceStrId() const;
		void setInstanceStrId(const QString& value);

		void setConfiguratorAddress1(const HostAddressPort& address);
		HostAddressPort configuratorAddress1() const;

		void setConfiguratorAddress2(const HostAddressPort& address);
		HostAddressPort configuratorAddress2() const;

	private:
		bool getArgumentFromXml(QDomElement& docElem, QString name, QString* result);
		bool getArgumentFromXml(QDomElement& docElem, QString name, int* result);

	private:
		QString m_instanceStrId;

		HostAddressPort m_cfgServiceAddress1;
		HostAddressPort m_cfgServiceAddress2;
	};
}
