#include "ScriptTuningClientApplication.h"
#include "Settings.h"

ScriptTuningClientApplication::ScriptTuningClientApplication()
	: QObject{}
{
}

QString ScriptTuningClientApplication::equipmentId() const
{
	return TuningClientAppSettings::instance().instanceStrId();
}

bool ScriptTuningClientApplication::start(QString program, QString arguments, QString workDir)
{
	return QProcess::startDetached(program, arguments.split(';', Qt::SkipEmptyParts), workDir);
}

void ScriptTuningClientApplication::setMainWindow(MainWindow* mainWindow)
{
	m_mainWindow = mainWindow;

	return;
}

MainWindow* ScriptTuningClientApplication::mainWindow()
{
	return m_mainWindow;
}

const MainWindow* ScriptTuningClientApplication::mainWindow() const
{
	return m_mainWindow;
}
