#include "TestSuiteSettings.h"
#include "../OnlineLib/SocketIO.h"

namespace TestSuite
{
	TestSuiteSettings::TestSuiteSettings():
		m_instanceStrId("SYSTEMID_WS00_TESTSUITE"),
		m_cfgServiceAddress1{"127.0.0.1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST},
		m_cfgServiceAddress2{"127.0.0.1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST}
	{
	}

	void TestSuiteSettings::restoreFromRegistry()
	{
		// read system settings
		//
		QSettings s;

		m_instanceStrId = s.value("m_instanceStrId", m_instanceStrId).toString();

		QString configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
		int configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();
		m_cfgServiceAddress1 = {configuratorIpAddress1, configuratorPort1};

		QString configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
		int configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();
		m_cfgServiceAddress2 = {configuratorIpAddress2, configuratorPort2};

//		m_loadScriptsFromPath = s.value("m_loadScriptsFromPath", false).toBool();
//		m_scriptsPath = s.value("m_scriptsPath", QString()).toString();

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

		QString parseError;
		int parseErrorLine = -1;
		int parseErrorColumn = -1;

		if (doc.setContent(&file, &parseError, &parseErrorLine, &parseErrorColumn) == false)
		{
			*errorMsg = QObject::tr("Failed to load contents of the file %1, error in line %2, column %3, message %4")
						.arg(fileName)
						.arg(parseErrorLine)
						.arg(parseErrorColumn)
						.arg(parseError);
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

//		// ScriptsPath
//		//
//		ok = getArgumentFromXml(docElem, "ScriptsPath", &m_scriptsPath);
//		if (ok == false)
//		{
//			*errorMsg = "Failed to read ScriptsPath argument from file!";
//			return false;
//		}
//		m_loadScriptsFromPath = m_scriptsPath.isEmpty() == false;

		return true;
	}

	void TestSuiteSettings::saveToRegistry()
	{
		QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

		s.setValue("m_instanceStrId", m_instanceStrId);

		s.setValue("m_configuratorIpAddress1", m_cfgServiceAddress1.addressStr());
		s.setValue("m_configuratorPort1", m_cfgServiceAddress1.port());

		s.setValue("m_configuratorIpAddress2", m_cfgServiceAddress2.addressStr());
		s.setValue("m_configuratorPort2", m_cfgServiceAddress2.port());

		//s.setValue("m_loadScriptsFromPath", m_loadScriptsFromPath);
		//s.setValue("m_scriptsPath", m_scriptsPath);

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

		//writer.writeComment("ScriptsPath (optional)");
		//writer.writeTextElement("ScriptsPath", "");

		writer.writeEndElement();	// TestSuiteConsoleArguments
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

//	bool TestSuiteSettings::loadScriptsFromPath() const
//	{
//		return m_loadScriptsFromPath;
//	}

//	void TestSuiteSettings::setLoadScriptsFromPath(bool value)
//	{
//		m_loadScriptsFromPath = value;
//	}

//	QString TestSuiteSettings::scriptsPath() const
//	{
//		return m_scriptsPath;
//	}

//	void TestSuiteSettings::setScriptsPath(const QString& path)
//	{
//		m_scriptsPath = path;
//	}
}
