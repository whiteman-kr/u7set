#include "ScriptTuningClientApplication.h"
#include "Settings.h"

ScriptTuningClientApplication::ScriptTuningClientApplication()
	: QObject{}
{
}

QString ScriptTuningClientApplication::equipmentId() const
{
	return theSettings.instanceStrId();
}

bool ScriptTuningClientApplication::start(QString program, QString arguments, QString workDir)
{
	return QProcess::startDetached(program, arguments.split(';', Qt::SkipEmptyParts), workDir);
}
