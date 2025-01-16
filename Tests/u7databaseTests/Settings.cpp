#include "Settings.h"

#include <iostream>
#include <QDomDocument>
#include <QFile>


bool getArgumentFromXml(QDomElement& docElem, QString name, QString* result)
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

bool getArgumentFromXml(QDomElement& docElem, QString name, int* result)
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

Settings::Settings()
{

}

const QString& Settings::databaseHost() const
{
	return m_databaseHost;
}

int Settings::databasePort() const
{
	return m_databasePort;
}

const QString& Settings::databaseUser() const
{
	return m_databaseUser;
}

const QString& Settings::databasePassword() const
{
	return m_databasePassword;
}

const QString& Settings::databaseAdministratorPassword() const
{
	return m_databaseAdministratorPassword;
}

const QString& Settings::projectAdministratorName() const
{
	return m_projectAdministratorName;
}

const QString& Settings::projectAdministratorPassword() const
{
	return m_projectAdministratorPassword;
}

const QString& Settings::fileManagementTestsProjectName() const
{
	return m_fileManagementTestsProjectName;
}

const QString& Settings::hardwareConfigurationTestsProjectName() const
{
	return m_hardwareConfigurationTestsProjectName;
}

const QString& Settings::signalTestsProjectName() const
{
	return m_signalTestsProjectName;
}

const QString& Settings::versionControlTestsProjectName() const
{
	return m_versionControlTestsProjectName;
}

const QString& Settings::dbTestsProjectName() const
{
	return m_dbTestsProjectName;
}


int Settings::loadConfigurationFile(const QString& fileName)
{
	QDomDocument doc("Document");

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QString errorMsg = QObject::tr("Failed to open configuration file %1.").arg(fileName);
		std::cout << errorMsg.toStdString() << std::endl;
		return 1;
	}

    if (static_cast<bool>(doc.setContent(&file)) == false)
	{
		QString errorMsg = QObject::tr("Failed to load contents of the file %1.").arg(fileName);
		std::cout << errorMsg.toStdString() << std::endl;
		file.close();
		return 1;
	}
	file.close();

	std::cout << "Configuration File: " << fileName.toStdString() << "\n";

	// Read and set task arguments
	//
	QDomElement docElem = doc.documentElement();

	// DatabaseAddress
	//
	bool ok = getArgumentFromXml(docElem, "DatabaseAddress", &m_databaseHost);
	if (ok == false)
	{
		std::cout << "Failed to read DatabaseAddress argument from file!" << std::endl;
		return 1;
	}
	if (m_databaseHost.isEmpty() == true)
	{
		std::cout << "DatabaseAddress argument can't be empty!" << std::endl;
		return 1;
	}

	// DatabasePort
	//
	ok = getArgumentFromXml(docElem, "DatabasePort", &m_databasePort);
	if (ok == false)
	{
		std::cout << "Failed to read DatabasePort argument from file!" << std::endl;
		return 1;
	}

	// DatabaseUserName
	//
	ok = getArgumentFromXml(docElem, "DatabaseUserName", &m_databaseUser);
	if (ok == false)
	{
		std::cout << "Failed to read DatabaseUserName argument from file!" << std::endl;
		return 1;
	}
	if (m_databaseUser.isEmpty() == true)
	{
		std::cout << "DatabaseUserName argument can't be empty!" << std::endl;
		return 1;
	}

	// DatabasePassword
	//
	ok = getArgumentFromXml(docElem, "DatabasePassword", &m_databasePassword);
	if (ok == false)
	{
		std::cout << "Failed to read DatabasePassword argument from file!" << std::endl;
		return 1;
	}
	if (m_databasePassword.isEmpty() == true)
	{
		std::cout << "DatabasePassword argument can't be empty!" << std::endl;
		return 1;
	}

	// DatabaseAdministratorPassword
	//
	ok = getArgumentFromXml(docElem, "DatabaseAdministratorPassword", &m_databaseAdministratorPassword);
	if (ok == false)
	{
		std::cout << "Failed to read DatabaseAdministratorPassword argument from file!" << std::endl;
		return 1;
	}
	if (m_databaseAdministratorPassword.isEmpty() == true)
	{
		std::cout << "DatabaseAdministratorPassword argument can't be empty!" << std::endl;
		return 1;
	}

	// ProjectAdministratorName
	//
	ok = getArgumentFromXml(docElem, "ProjectAdministratorName", &m_projectAdministratorName);
	if (ok == false)
	{
		std::cout << "Failed to read ProjectAdministratorName argument from file!" << std::endl;
		return 1;
	}
	if (m_projectAdministratorName.isEmpty() == true)
	{
		std::cout << "ProjectAdministratorName argument can't be empty!" << std::endl;
		return 1;
	}

	// ProjectAdministratorPassword
	//
	ok = getArgumentFromXml(docElem, "ProjectAdministratorPassword", &m_projectAdministratorPassword);
	if (ok == false)
	{
		std::cout << "Failed to read ProjectAdministratorPassword argument from file!" << std::endl;
		return 1;
	}
	if (m_projectAdministratorPassword.isEmpty() == true)
	{
		std::cout << "ProjectAdministratorPassword argument can't be empty!" << std::endl;
		return 1;
	}

	// SignalTestsProjectName
	//
	ok = getArgumentFromXml(docElem, "SignalTestsProjectName", &m_signalTestsProjectName);
	if (ok == false)
	{
		std::cout << "Failed to read SignalTestsProjectName argument from file!" << std::endl;
		return 1;
	}
	if (m_signalTestsProjectName.isEmpty() == true)
	{
		std::cout << "SignalTestsProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	// DbTestsProjectName
	//
	ok = getArgumentFromXml(docElem, "DbTestsProjectName", &m_dbTestsProjectName);
	if (ok == false)
	{
		std::cout << "Failed to read DbTestsProjectName argument from file!" << std::endl;
		return 1;
	}
	if (m_dbTestsProjectName.isEmpty() == true)
	{
		std::cout << "DbTestsProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	// FileManagementTestsProjectName
	//
	ok = getArgumentFromXml(docElem, "FileManagementTestsProjectName", &m_fileManagementTestsProjectName);
	if (ok == false)
	{
		std::cout << "Failed to read FileManagementTestsProjectName argument from file!" << std::endl;
		return 1;
	}
	if (m_fileManagementTestsProjectName.isEmpty() == true)
	{
		std::cout << "FileManagementTestsProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	// HardwareConfigurationTestsProjectName
	//
	ok = getArgumentFromXml(docElem, "HardwareConfigurationTestsProjectName", &m_hardwareConfigurationTestsProjectName);
	if (ok == false)
	{
		std::cout << "Failed to read HardwareConfigurationTestsProjectName argument from file!" << std::endl;
		return 1;
	}
	if (m_hardwareConfigurationTestsProjectName.isEmpty() == true)
	{
		std::cout << "HardwareConfigurationTestsProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	// VersionControlTestsProjectName
	//
	ok = getArgumentFromXml(docElem, "VersionControlTestsProjectName", &m_versionControlTestsProjectName);
	if (ok == false)
	{
		std::cout << "Failed to read VersionControlTestsProjectName argument from file!" << std::endl;
		return 1;
	}
	if (m_versionControlTestsProjectName.isEmpty() == true)
	{
		std::cout << "VersionControlTestsProjectName argument can't be empty!" << std::endl;
		return 1;
	}

	std::cout << "DatabaseAddress: " << m_databaseHost.toStdString() << "\n";
	std::cout << "DatabasePort: " << m_databasePort << "\n";
	std::cout << "DatabaseUserName: " << m_databaseUser.toStdString() << "\n";
	std::cout << "DatabasePassword: " << m_databasePassword.toStdString() << "\n";
	std::cout << "DatabaseAdministratorPassword: " << m_databaseAdministratorPassword.toStdString() << "\n";
	std::cout << "ProjectAdministratorName: " << m_projectAdministratorName.toStdString() << "\n";
	std::cout << "ProjectAdministratorPassword: " << m_projectAdministratorPassword.toStdString() << "\n";
	std::cout << "SignalTestsProjectName: " << m_signalTestsProjectName.toStdString() << "\n";
	std::cout << "DbTestsProjectName: " << m_dbTestsProjectName.toStdString() << "\n";
	std::cout << "FileManagementTestsProjectName: " << m_fileManagementTestsProjectName.toStdString() << "\n";
	std::cout << "HardwareConfigurationTestsProjectName: " << m_hardwareConfigurationTestsProjectName.toStdString() << "\n";
	std::cout << "VersionControlTestsProjectName: " << m_versionControlTestsProjectName.toStdString() << "\n";

	return 0;
}

Settings theSettings;
