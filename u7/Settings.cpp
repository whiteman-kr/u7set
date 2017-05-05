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
	m_buildOutputPath(QDir().toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DataLocation))),
	m_expertMode(false),
	#ifdef Q_OS_LINUX
		m_configuratorSerialPort("ttyS0")
	#endif
	#ifdef Q_OS_WIN32
		m_configuratorSerialPort("\\\\.\\COM3")
	#endif
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
    s.setValue("EquipmentTabPage/PropertiesSplitter/state", m_equipmentTabPagePropertiesSplitterState);

	s.setValue("BuildTabPage/Splitter/state", m_buildTabPageSplitterState);

    s.setValue("TextEditorProperties/pos", m_DialogTextEditorWindowPos);
    s.setValue("TextEditorProperties/geometry", m_DialogTextEditorWindowGeometry);

	s.setValue("PropertyEditor/multiLinePos", m_multiLinePropertyEditorWindowPos);
	s.setValue("PropertyEditor/multiLineGeometry", m_multiLinePropertyEditorGeometry);

	s.setValue("PropertyEditor/scriptHelpPos", m_scriptHelpWindowPos);
	s.setValue("PropertyEditor/scriptHelpGeometry", m_scriptHelpWindowGeometry);

	s.setValue("LoginDialog/loginCompleter", m_loginCompleter);

	s.setValue("ConnectionEditor/pos", m_connectionEditorWindowPos);
	s.setValue("ConnectionEditor/geometry", m_connectionEditorWindowGeometry);
    s.setValue("ConnectionEditor/splitter", m_connectionEditorSplitterState);
    s.setValue("ConnectionEditor/peSplitterPos", m_connectionEditorPeSplitterPosition);

	s.setValue("ConnectionEditor/sortColumn", m_connectionEditorSortColumn);
	s.setValue("ConnectionEditor/sortOrder", static_cast<int>(m_connectionEditorSortOrder));

	s.setValue("ConnectionEditor/masks", m_connectionEditorMasks);

	s.setValue("CreateSchema/lastSelectedLmDescriptionFile", m_lastSelectedLmDescriptionFile);

	s.setValue("SchemaItem/pos", m_schemaItemPropertiesWindowPos);
	s.setValue("SchemaItem/geometry", m_schemaItemPropertiesWindowGeometry);
	s.setValue("SchemaItem/Splitter/state", m_schemaItemSplitterState);

	s.setValue("Main/m_expertMode", m_expertMode);

	s.setValue("m_infoMode", m_infoMode);

	s.setValue("UploadTabPage/Splitter/state", m_UploadTabPageSplitterState);

	s.setValue("BuildTabPage/m_buildWarningLevel", m_buildWarningLevel);
	s.setValue("BuildTabPage/m_buildSerachCompleter", m_buildSerachCompleter);

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

    m_equipmentTabPagePropertiesSplitterState = s.value("EquipmentTabPage/PropertiesSplitter/state").toInt();
    if (m_equipmentTabPagePropertiesSplitterState < 150)
	{
        m_equipmentTabPagePropertiesSplitterState = 150;
	}

    m_buildTabPageSplitterState = s.value("BuildTabPage/Splitter/state").toByteArray();

    m_DialogTextEditorWindowPos = s.value("TextEditorProperties/pos", QPoint(-1, -1)).toPoint();
    m_DialogTextEditorWindowGeometry = s.value("TextEditorProperties/geometry").toByteArray();

	m_multiLinePropertyEditorWindowPos = s.value("PropertyEditor/multiLinePos", QPoint(-1, -1)).toPoint();
	m_multiLinePropertyEditorGeometry = s.value("PropertyEditor/multiLineGeometry").toByteArray();

	m_scriptHelpWindowPos = s.value("PropertyEditor/scriptHelpPos", QPoint(-1, -1)).toPoint();
	m_scriptHelpWindowGeometry = s.value("PropertyEditor/scriptHelpGeometry").toByteArray();

	m_loginCompleter = s.value("LoginDialog/loginCompleter").toStringList();
	if (m_loginCompleter.isEmpty() == true)
	{
		m_loginCompleter << "Administrator";
	}

    //

	m_connectionEditorWindowPos = s.value("ConnectionEditor/pos", QPoint(-1, -1)).toPoint();
	m_connectionEditorWindowGeometry = s.value("ConnectionEditor/geometry").toByteArray();
    m_connectionEditorSplitterState = s.value("ConnectionEditor/splitter").toByteArray();
    m_connectionEditorPeSplitterPosition = s.value("ConnectionEditor/peSplitterPos").toInt();
    if (m_connectionEditorPeSplitterPosition < 150)
    {
        m_connectionEditorPeSplitterPosition = 150;
    }

	m_connectionEditorSortColumn = s.value("ConnectionEditor/sortColumn").toInt();
	m_connectionEditorSortOrder = static_cast<Qt::SortOrder>(s.value("ConnectionEditor/sortOrder").toInt());
	m_connectionEditorMasks = s.value("ConnectionEditor/masks").toStringList();

    //
	m_lastSelectedLmDescriptionFile = s.value("CreateSchema/lastSelectedLmDescriptionFile", "").toString();

	m_schemaItemPropertiesWindowPos = s.value("SchemaItem/pos", QPoint(-1, -1)).toPoint();
	m_schemaItemPropertiesWindowGeometry = s.value("SchemaItem/geometry").toByteArray();

	m_schemaItemSplitterState = s.value("SchemaItem/Splitter/state").toInt();
    if (m_schemaItemSplitterState < 150)
	{
        m_schemaItemSplitterState = 150;
	}

	m_expertMode = s.value("Main/m_expertMode", false).toBool();

	m_infoMode = s.value("m_infoMode").toBool();

	m_UploadTabPageSplitterState = s.value("UploadTabPage/Splitter/state").toByteArray();

	m_buildWarningLevel = s.value("BuildTabPage/m_buildWarningLevel").toBool();
	m_buildSerachCompleter = s.value("BuildTabPage/m_buildSerachCompleter").toStringList();

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
	s.setValue("m_configuratorSerialPort", m_configuratorSerialPort);
	s.setValue("m_configuratorShowDebugInfo", m_configuratorShowDebugInfo);
	s.setValue("m_configuratorVerify", m_configuratorVerify);

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
	m_configuratorSerialPort = s.value("m_configuratorSerialPort", m_configuratorSerialPort).toString();
	m_configuratorShowDebugInfo = s.value("m_configuratorShowDebugInfo", m_configuratorShowDebugInfo).toBool();
	m_configuratorVerify = s.value("m_configuratorVerify", m_configuratorVerify).toBool();

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

void Settings::setDebugMode(bool value)
{
	m_debugMode = value;
}

bool Settings::debugMode() const
{
	return m_debugMode;
}

bool Settings::isDebugMode() const
{
	return debugMode();
}

bool Settings::isExpertMode() const
{
	return m_expertMode;
}

void Settings::setExpertMode(bool value)
{
	m_expertMode = value;
}

bool Settings::isInfoMode() const
{
	return m_infoMode;
}

bool Settings::infoMode() const
{
	return m_infoMode;
}

void Settings::setInfoMode(bool value)
{
	m_infoMode = value;
}

int Settings::buildWarningLevel() const
{
	return m_buildWarningLevel;
}

void Settings::setBuildWarningLevel(int value)
{
	m_buildWarningLevel = value;
}


const QStringList& Settings::buildSearchCompleter() const
{
	return m_buildSerachCompleter;
}

QStringList& Settings::buildSearchCompleter()
{
	return m_buildSerachCompleter;
}
