#include "IdePropertyEditor.h"
#include "Settings.h"

IdePropertyEditor::IdePropertyEditor(QWidget* parent) :
	PropertyEditor(parent)
{
	// Set script help data
	//
	QFile file(":/ScriptHelp/scripthelp.html");

	if (file.open(QIODevice::ReadOnly) == true)
	{
		QByteArray data = file.readAll();
		if (data.size() > 0)
		{
			setScriptHelp(QString::fromUtf8(data));
		}
	}

	setScriptHelpWindowPos(theSettings.m_scriptHelpWindowPos);
	setScriptHelpWindowGeometry(theSettings.m_scriptHelpWindowGeometry);
}

IdePropertyEditor::~IdePropertyEditor()
{
}

void IdePropertyEditor::saveSettings()
{
	theSettings.m_scriptHelpWindowPos = scriptHelpWindowPos();
	theSettings.m_scriptHelpWindowGeometry = scriptHelpWindowGeometry();
}
