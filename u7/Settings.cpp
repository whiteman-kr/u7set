#include "../u7/Settings.h"
#include "QStandardPaths"
#include "QDir"
#include <QSettings>

Settings theSettings;

Settings::Settings() :
	m_serverIpAddress("127.0.0.1"),
	m_serverPort(0),
	m_serverUsername("u7"),
	m_serverPassword("P2ssw0rd"),
	m_buildOutputPath(QDir().toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DataLocation)))

{
}

Settings::~Settings()
{
}

void Settings::write() const
{
	writeUserScope();
	writeSystemScope();
}

void Settings::load()
{
	loadUserScope();
	loadSystemScope();
}

void Settings::writeUserScope() const
{
	QSettings s;

	s.setValue("MainWindow/pos", m_mainWindowPos);
	s.setValue("MainWindow/geometry", m_mainWindowGeometry);
	s.setValue("MainWindow/state", m_mainWindowState);

	s.setValue("loginDialog_defaultUsername", loginDialog_defaultUsername);

	s.setValue("ConfigurationTabPage/Splitter/state", m_configurationTabPageSplitterState);

	s.setValue("EquipmentTabPage/Splitter/state", m_equipmentTabPageSplitterState);

	s.setValue("BuildTabPage/Splitter/state", m_buildTabPageSplitterState);

    s.setValue("AFBLEditor/pos", m_abflEditorWindowPos);
    s.setValue("AFBLEditor/geometry", m_abflEditorWindowGeometry);

    s.setValue("AFBLProperties/pos", m_abflPropertiesWindowPos);
    s.setValue("AFBLProperties/geometry", m_abflPropertiesWindowGeometry);

    s.setValue("TextEditorProperties/pos", m_DialogTextEditorWindowPos);
    s.setValue("TextEditorProperties/geometry", m_DialogTextEditorWindowGeometry);

	s.setValue("PropertyEditor/multiLinePos", m_multiLinePropertyEditorWindowPos);
	s.setValue("PropertyEditor/multiLineGeometry", m_multiLinePropertyEditorGeometry);

	s.setValue("LoginDialog/loginCompleter", m_loginCompleter);

	return;
}
void Settings::loadUserScope()
{
	QSettings s;

	m_mainWindowPos = s.value("MainWindow/pos", QPoint(200, 200)).toPoint();
	m_mainWindowGeometry = s.value("MainWindow/geometry").toByteArray();
	m_mainWindowState = s.value("MainWindow/state").toByteArray();

	loginDialog_defaultUsername = s.value("loginDialog_defaultUsername").toString();

	m_configurationTabPageSplitterState = s.value("ConfigurationTabPage/Splitter/state").toByteArray();

	m_equipmentTabPageSplitterState = s.value("EquipmentTabPage/Splitter/state").toByteArray();

	m_buildTabPageSplitterState = s.value("BuildTabPage/Splitter/state").toByteArray();

    m_abflEditorWindowPos = s.value("AFBLEditor/pos", QPoint(-1, -1)).toPoint();
    m_abflEditorWindowGeometry = s.value("AFBLEditor/geometry").toByteArray();

    m_abflPropertiesWindowPos = s.value("AFBLProperties/pos", QPoint(-1, -1)).toPoint();
    m_abflPropertiesWindowGeometry = s.value("AFBLProperties/geometry").toByteArray();

    m_DialogTextEditorWindowPos = s.value("TextEditorProperties/pos", QPoint(-1, -1)).toPoint();
    m_DialogTextEditorWindowGeometry = s.value("TextEditorProperties/geometry").toByteArray();

	m_multiLinePropertyEditorWindowPos = s.value("PropertyEditor/multiLinePos", QPoint(-1, -1)).toPoint();
	m_multiLinePropertyEditorGeometry = s.value("PropertyEditor/multiLineGeometry").toByteArray();

	m_loginCompleter = s.value("LoginDialog/loginCompleter").toStringList();

	if (m_loginCompleter.isEmpty() == true)
	{
		m_loginCompleter << "Administrator";
	}

    return;
}

void Settings::writeSystemScope() const
{
	QSettings s;

	s.setValue("m_serverIpAddress", m_serverIpAddress);
	s.setValue("m_serverPort", m_serverPort);
	s.setValue("m_serverUsername", m_serverUsername);
	s.setValue("m_serverPassword", m_serverPassword);
	s.setValue("m_buildOutputPath", m_buildOutputPath);

	return;
}
void Settings::loadSystemScope()
{
	QSettings s;

	m_serverIpAddress = s.value("m_serverIpAddress", "127.0.0.1").toString();
	m_serverPort = s.value("m_serverPort", 5432).toInt();
	m_serverUsername = s.value("m_serverUsername", "u7").toString();
	m_serverPassword = s.value("m_serverPassword", "P2ssw0rd").toString();
	m_buildOutputPath = s.value("m_buildOutputPath", m_buildOutputPath).toString();

	return;
}

const QString& Settings::serverIpAddress() const
{
	return m_serverIpAddress;
}
void Settings::setServerIpAddress(const QString& value)
{
	m_serverIpAddress = value;
}

int Settings::serverPort() const
{
	return m_serverPort;
}
void Settings::setServerPort(int value)
{
	m_serverPort = value;
}

const QString& Settings::serverUsername() const
{
	return m_serverUsername;
}
void Settings::setServerUsername(const QString& value)
{
	m_serverUsername = value;
}

const QString& Settings::serverPassword() const
{
	return m_serverPassword;
}
void Settings::setServerPassword(const QString& value)
{
	m_serverPassword = value;
}


const QString& Settings::buildOutputPath() const
{
	return m_buildOutputPath;
}

void Settings::setBuildOutputPath(const QString& value)
{
	m_buildOutputPath = value;
}

const QStringList& Settings::loginCompleter() const
{
	return m_loginCompleter;
}

QStringList& Settings::loginCompleter()
{
	return m_loginCompleter;
}
