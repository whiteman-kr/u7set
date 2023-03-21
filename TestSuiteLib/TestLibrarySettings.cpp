#include "TestLibrarySettings.h"
#include "../OnlineLib/SocketIO.h"

TestLibrarySettings::TestLibrarySettings():
	m_instanceStrId("SYSTEMID_WS00_TESTSUITE"),
	m_configuratorIpAddress1("127.0.0.1"),
	m_configuratorPort1(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
	m_configuratorIpAddress2("127.0.0.1"),
	m_configuratorPort2(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST)
{

}

void TestLibrarySettings::restoreFromRegistry()
{
	// read system settings
	//
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	m_instanceStrId = s.value("m_instanceStrId", m_instanceStrId).toString();

	m_configuratorIpAddress1 = s.value("m_configuratorIpAddress1", "127.0.0.1").toString();
	m_configuratorPort1 = s.value("m_configuratorPort1", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_configuratorIpAddress2 = s.value("m_configuratorIpAddress2", "127.0.0.1").toString();
	m_configuratorPort2 = s.value("m_configuratorPort2", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST).toInt();

	m_loadScriptsFromPath = s.value("m_loadScriptsFromPath", false).toBool();
	m_scriptsPath = s.value("m_scriptsPath", QString()).toString();

}

bool TestLibrarySettings::restoreFromFile(const QString& fileName, QString* errorMsg)
{
	// Read arguments from XML document
	//
	QDomDocument doc("Document");

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		*errorMsg = QObject::tr("Failed to open file %1.").arg(fileName);
		return false;
	}


	if (doc.setContent(&file) == false)
	{
		*errorMsg = QObject::tr("Failed to load contents of the file %1.").arg(fileName);
		return false;
	}
	file.close();

	// Read and set task arguments
	//
	QDomElement docElem = doc.documentElement();


	// DatabaseAddress
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
	ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress1", &m_configuratorIpAddress1);
	if (ok == false)
	{
		*errorMsg = "Failed to read ConfiguratorIPAddress1 argument from file!";
		return false;
	}
	if (m_configuratorIpAddress1.isEmpty() == true)
	{
		*errorMsg = "ConfiguratorIPAddress1 argument can't be empty!";
		return false;
	}

	// ConfiguratorPort1
	//
	ok = getArgumentFromXml(docElem, "ConfiguratorPort1", &m_configuratorPort1);
	if (ok == false)
	{
		*errorMsg = "Failed to read ConfiguratorPort1 argument from file!";
		return false;
	}

	// ConfiguratorIPAddress2
	//
	ok = getArgumentFromXml(docElem, "ConfiguratorIPAddress2", &m_configuratorIpAddress2);
	if (ok == false)
	{
		*errorMsg = "Failed to read ConfiguratorIPAddress2 argument from file!";
		return false;
	}
	if (m_configuratorIpAddress2.isEmpty() == true)
	{
		*errorMsg = "ConfiguratorIPAddress2 argument can't be empty!";
		return false;
	}

	// ConfiguratorPort2
	//
	ok = getArgumentFromXml(docElem, "ConfiguratorPort2", &m_configuratorPort2);
	if (ok == false)
	{
		*errorMsg = "Failed to read ConfiguratorPort2 argument from file!";
		return false;
	}

	// ScriptsPath
	//
	ok = getArgumentFromXml(docElem, "ScriptsPath", &m_scriptsPath);
	if (ok == false)
	{
		*errorMsg = "Failed to read ScriptsPath argument from file!";
		return false;
	}
	m_loadScriptsFromPath = m_scriptsPath.isEmpty() == false;

	return true;
}

void TestLibrarySettings::saveToRegistry()
{
	QSettings s(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	s.setValue("m_instanceStrId", m_instanceStrId);

	s.setValue("m_configuratorIpAddress1", m_configuratorIpAddress1);
	s.setValue("m_configuratorPort1", m_configuratorPort1);

	s.setValue("m_configuratorIpAddress2", m_configuratorIpAddress2);
	s.setValue("m_configuratorPort2", m_configuratorPort2);

	s.setValue("m_loadScriptsFromPath", m_loadScriptsFromPath);
	s.setValue("m_scriptsPath", m_scriptsPath);
}

bool TestLibrarySettings::getArgumentFromXml(QDomElement& docElem, QString name, QString* result)
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

bool TestLibrarySettings::getArgumentFromXml(QDomElement& docElem, QString name, int* result)
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

bool TestLibrarySettings::createTemplateConfigurationFile(const QString& fileName)
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
	writer.writeTextElement("ConfiguratorPort1", "13312");

	writer.writeComment("Configurator IP Address 2");
	writer.writeTextElement("ConfiguratorIPAddress2", "127.0.0.1");

	writer.writeComment("Configurator Port 2");
	writer.writeTextElement("ConfiguratorPort2", "13312");

	writer.writeComment("ScriptsPath (optional)");
	writer.writeTextElement("ScriptsPath", "");

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

QString TestLibrarySettings::instanceStrId() const
{
	return m_instanceStrId;
}

void TestLibrarySettings::setInstanceStrId(const QString& value)
{
	m_instanceStrId = value;
}

HostAddressPort TestLibrarySettings::configuratorAddress1() const
{
	return HostAddressPort(m_configuratorIpAddress1, m_configuratorPort1);
}

void TestLibrarySettings::setConfiguratorAddress1(const QString& address, int port)
{
	m_configuratorIpAddress1 = address;
	m_configuratorPort1 = port;
}

HostAddressPort TestLibrarySettings::configuratorAddress2() const
{
	return HostAddressPort(m_configuratorIpAddress2, m_configuratorPort2);
}

void TestLibrarySettings::setConfiguratorAddress2(const QString& address, int port)
{
	m_configuratorIpAddress2 = address;
	m_configuratorPort2 = port;
}

bool TestLibrarySettings::loadScriptsFromPath() const
{
	return m_loadScriptsFromPath;
}

void TestLibrarySettings::setLoadScriptsFromPath(bool value)
{
	m_loadScriptsFromPath = value;
}

QString TestLibrarySettings::scriptsPath() const
{
	return m_scriptsPath;
}

void TestLibrarySettings::setScriptsPath(const QString& path)
{
	m_scriptsPath = path;
}
