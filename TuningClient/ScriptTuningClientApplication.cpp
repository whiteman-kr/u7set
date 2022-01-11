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
