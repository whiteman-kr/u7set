#include <TestSuiteLib/TestSuiteSettings.h>

#include "../OnlineLib/SocketIO.h"

#include <QDomDocument>
#include <QFile>
#include <QSettings>
#include <QXmlStreamWriter>

namespace TestSuite
{
	TestSuiteSettings::TestSuiteSettings() :
		m_instanceStrId("SYSTEMID_WS00_TESTSUITE"),
		m_cfgServiceAddress1{"127.0.0.1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST},
		m_cfgServiceAddress2{"127.0.0.1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST}
	{
	}

	void TestSuiteSettings::restoreFromRegistry(const QSettings& s)
	{
		// read system settings
		//
		m_instanceStrId = s.value("TestSuiteSettings/m_instanceStrId", m_instanceStrId).toString();

		QString configuratorIpAddress1 = s.value("TestSuiteSettings/m_configuratorIpAddress1", "127.0.0.1").toString();
		int configuratorPort1 = s.value("TestSuiteSettings/m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();
		m_cfgServiceAddress1 = {configuratorIpAddress1, configuratorPort1};

		QString configuratorIpAddress2 = s.value("TestSuiteSettings/m_configuratorIpAddress2", "127.0.0.1").toString();
		int configuratorPort2 = s.value("TestSuiteSettings/m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();
		m_cfgServiceAddress2 = {configuratorIpAddress2, configuratorPort2};

		return;
	}

	bool TestSuiteSettings::restoreFromFile(const QString& fileName, QString* errorMsg)
	{
		Q_ASSERT(errorMsg);

		// Read arguments from XML document
		//
		QDomDocument doc("Document");

		QFile file(fileName);
		if (file.open(QIODevice::ReadOnly) == false)
		{
			*errorMsg = QObject::tr("Failed to open file %1.").arg(fileName);
			return false;
		}

		QDomDocument::ParseResult pr = doc.setContent(&file);

		if (pr.errorMessage.isEmpty() == false)
		{
			*errorMsg = QObject::tr("Failed to load contents of the file %1, error in line %2, column %3, message %4")
							.arg(fileName)
							.arg(pr.errorLine)
							.arg(pr.errorColumn)
							.arg(pr.errorMessage);
			return false;
		}

		file.close();

		// Read and set task arguments
		//
		QDomElement docElem = doc.documentElement();

		// InstanceStrID
		//
		bool ok = getArgumentFromXml(docElem, "InstanceStrID", &m_instanceStrId);
		if (ok == false)
		{
			*errorMsg = "Failed to read InstanceStrID argument from file!";
			return false;
		}

		if (m_instanceStrId.isEmpty() == true)
		{
			*errorMsg = "InstanceStrID argument can't be empty!";
			return false;
		}

		// ConfiguratorIPAddress1
		//
		QString configuratorIpAddress1;

		ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress1", &configuratorIpAddress1);
		if (ok == false)
		{
			*errorMsg = "Failed to read ConfiguratorIPAddress1 argument from file!";
			return false;
		}

		if (configuratorIpAddress1.isEmpty() == true)
		{
			*errorMsg = "ConfiguratorIPAddress1 argument can't be empty!";
			return false;
		}

		// ConfiguratorPort1
		//
		int configuratorPort1 = 0;

		ok = getArgumentFromXml(docElem, "ConfiguratorPort1", &configuratorPort1);
		if (ok == false)
		{
			*errorMsg = "Failed to read ConfiguratorPort1 argument from file!";
			return false;
		}

		m_cfgServiceAddress1 = {configuratorIpAddress1, configuratorPort1};

		// ConfiguratorIPAddress2
		//
		QString configuratorIpAddress2;

		ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress2", &configuratorIpAddress2);
		if (ok == false)
		{
			*errorMsg = "Failed to read ConfiguratorIPAddress2 argument from file!";
			return false;
		}

		if (configuratorIpAddress2.isEmpty() == true)
		{
			*errorMsg = "ConfiguratorIPAddress2 argument can't be empty!";
			return false;
		}

		// ConfiguratorPort2
		//
		int configuratorPort2 = 0;

		ok = getArgumentFromXml(docElem, "ConfiguratorPort2", &configuratorPort2);
		if (ok == false)
		{
			*errorMsg = "Failed to read ConfiguratorPort2 argument from file!";
			return false;
		}

		m_cfgServiceAddress2 = {configuratorIpAddress2, configuratorPort2};

		return true;
	}

	void TestSuiteSettings::saveToRegistry(QSettings& s) const
	{
		s.setValue("TestSuiteSettings/m_instanceStrId", m_instanceStrId);

		s.setValue("TestSuiteSettings/m_configuratorIpAddress1", m_cfgServiceAddress1.addressStr());
		s.setValue("TestSuiteSettings/m_configuratorPort1", m_cfgServiceAddress1.port());

		s.setValue("TestSuiteSettings/m_configuratorIpAddress2", m_cfgServiceAddress2.addressStr());
		s.setValue("TestSuiteSettings/m_configuratorPort2", m_cfgServiceAddress2.port());

		return;
	}

	bool TestSuiteSettings::getArgumentFromXml(QDomElement& docElem, QString name, QString* result)
	{
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return false;
		}

		QDomNodeList softwareNodes = docElem.elementsByTagName(name);
		if (softwareNodes.size() != 1)
		{
			return false;
		}

		QDomElement elem = softwareNodes.item(0).toElement();
		*result = elem.text();

		return true;
	}

	bool TestSuiteSettings::getArgumentFromXml(QDomElement& docElem, QString name, int* result)
	{
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return false;
		}

		QString str;
		if (getArgumentFromXml(docElem, name, &str) == false)
		{
			return false;
		}

		bool ok = false;
		*result = str.toInt(&ok);

		return ok;
	}

	bool TestSuiteSettings::createTemplateSettingsFile(const QString& fileName)
	{
		QByteArray data;
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);
		writer.writeStartDocument();
		writer.writeStartElement("TestSuiteConsoleArguments");

		writer.writeComment("TestSuite InstanceStrID");
		writer.writeTextElement("InstanceStrID", "SYSTEMID_RACKID_WS00_TESTSUITE");

		writer.writeComment("Configurator IP Address 1");
		writer.writeTextElement("ConfiguratorIPAddress1", "127.0.0.1");

		writer.writeComment("Configurator Port 1");
		writer.writeTextElement("ConfiguratorPort1", QString::number(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST));

		writer.writeComment("Configurator IP Address 2");
		writer.writeTextElement("ConfiguratorIPAddress2", "127.0.0.1");

		writer.writeComment("Configurator Port 2");
		writer.writeTextElement("ConfiguratorPort2", QString::number(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST));

		writer.writeEndElement(); // TestSuiteConsoleArguments
		writer.writeEndDocument();

		QFile f(fileName);
		if (f.open(QFile::WriteOnly) == false)
		{
			return false;
		}

		f.write(data);

		return true;
	}

	QString TestSuiteSettings::instanceStrId() const
	{
		return m_instanceStrId;
	}

	void TestSuiteSettings::setInstanceStrId(const QString& value)
	{
		m_instanceStrId = value;
	}

	void TestSuiteSettings::setConfiguratorAddress1(const HostAddressPort& address)
	{
		m_cfgServiceAddress1 = address;
	}

	HostAddressPort TestSuiteSettings::configuratorAddress1() const
	{
		return m_cfgServiceAddress1;
	}

	void TestSuiteSettings::setConfiguratorAddress2(const HostAddressPort& address)
	{
		m_cfgServiceAddress2 = address;
	}

	HostAddressPort TestSuiteSettings::configuratorAddress2() const
	{
		return m_cfgServiceAddress2;
	}
} // namespace TestSuite
