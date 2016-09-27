#include "DialogPresetProperties.h"
#include "Settings.h"

//
//
// DialogPresetProperties
//
//

DialogPresetProperties::DialogPresetProperties(std::shared_ptr<PropertyObject> object, QWidget *parent)
	:PropertyEditorDialog(object, parent)
{
	setWindowTitle(tr("Preset Properties"));

	if (theSettings.m_presetPropertiesWindowPos.x() != -1 && theSettings.m_presetPropertiesWindowPos.y() != -1)
	{
		setSplitterPosition(theSettings.m_presetPropertiesSplitterState);
		move(theSettings.m_presetPropertiesWindowPos);
		restoreGeometry(theSettings.m_presetPropertiesWindowGeometry);
	}
}

DialogPresetProperties::~DialogPresetProperties()
{
}

bool DialogPresetProperties::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{

	return true;
}

void DialogPresetProperties::closeEvent(QCloseEvent * e)
{
	Q_UNUSED(e);
	saveSettings();

}

void DialogPresetProperties::done(int r)
{
	saveSettings();
	PropertyEditorDialog::done(r);
}

void DialogPresetProperties::saveSettings()
{
	theSettings.m_presetPropertiesSplitterState = splitterPosition();
	theSettings.m_presetPropertiesWindowPos = pos();
	theSettings.m_presetPropertiesWindowGeometry = saveGeometry();
}
