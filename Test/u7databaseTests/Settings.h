#pragma once

class Settings
{
public:
	Settings();

	const QString& databaseHost() const;
	int databasePort() const;
	const QString &databaseUser() const;
	const QString &databasePassword() const;

	const QString& databaseAdministratorPassword() const;

	const QString& projectAdministratorName() const;
	const QString& projectAdministratorPassword() const;

	const QString& signalTestsProjectName() const;
	const QString& dbTestsProjectName() const;
	const QString& fileManagementTestsProjectName() const;
	const QString& hardwareConfigurationTestsProjectName() const;
	const QString& versionControlTestsProjectName() const;

	int loadConfigurationFile(const QString& fileName);

private:
	QString m_databaseHost = "127.0.0.1";
	int m_databasePort = 5432;
	QString m_databaseUser = "u7";
	QString m_databasePassword = "P2ssw0rd";

	QString m_databaseAdministratorPassword = "P2ssw0rd";	// password for "postgres" user

	QString m_projectAdministratorName = "Administrator";
	QString m_projectAdministratorPassword = "P2ssw0rd";

	QString m_signalTestsProjectName = "signalstests";
	QString m_dbTestsProjectName = "dbtests";
	QString m_fileManagementTestsProjectName = "dbfiletests";
	QString m_hardwareConfigurationTestsProjectName = "dbbuildtests";
	QString m_versionControlTestsProjectName = "dbversiontests";

};

extern Settings theSettings;


