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

	if (TuningClientAppSettings::instance().user().m_presetPropertiesWindowPos.x() != -1 && 
		TuningClientAppSettings::instance().user().m_presetPropertiesWindowPos.y() != -1)
	{
		setSplitterPosition(TuningClientAppSettings::instance().user().m_presetPropertiesSplitterState);
		move(TuningClientAppSettings::instance().user().m_presetPropertiesWindowPos);
		restoreGeometry(TuningClientAppSettings::instance().user().m_presetPropertiesWindowGeometry);
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
	TuningClientAppSettings::instance().user().m_presetPropertiesSplitterState = splitterPosition();
	TuningClientAppSettings::instance().user().m_presetPropertiesWindowPos = pos();
	TuningClientAppSettings::instance().user().m_presetPropertiesWindowGeometry = saveGeometry();
}
