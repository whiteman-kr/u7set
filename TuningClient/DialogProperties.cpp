#include "DialogProperties.h"
#include "Settings.h"

//
//
// DialogProperties
//
//

DialogProperties::DialogProperties(QWidget* parent)
	: PropertyEditorDialog(parent)
{
	setWindowTitle(tr("Properties"));

	if (theSettings.m_presetPropertiesWindowPos.x() != -1 && theSettings.m_presetPropertiesWindowPos.y() != -1)
	{
		setSplitterPosition(theSettings.m_presetPropertiesSplitterState);
		move(theSettings.m_presetPropertiesWindowPos);
		restoreGeometry(theSettings.m_presetPropertiesWindowGeometry);
	}
}

DialogProperties::~DialogProperties()
{
}

void DialogProperties::closeEvent(QCloseEvent* e)
{
	Q_UNUSED(e);
	saveSettings();

}

void DialogProperties::done(int r)
{
	saveSettings();
	PropertyEditorDialog::done(r);
}

void DialogProperties::saveSettings()
{
	theSettings.m_presetPropertiesSplitterState = splitterPosition();
	theSettings.m_presetPropertiesWindowPos = pos();
	theSettings.m_presetPropertiesWindowGeometry = saveGeometry();
}
