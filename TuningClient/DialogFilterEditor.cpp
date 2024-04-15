#include "DialogFilterEditor.h"
#include "Settings.h"
#include "MainWindow.h"


DialogFilterEditor::DialogFilterEditor(ClientLib::TuningSignalManager& tuningSignalManager, TuningFilterStorage& filterStorage, QWidget* parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningSignalManager(tuningSignalManager)
{
	setWindowTitle(tr("Filters"));

	m_tuningFilterEditor = new TuningFilterEditor(filterStorage,
												  tuningSignalManager,
												  false,	/*readOnly*/
												  true,		/*setCurrentEnabled*/
												  true,		/*typeTreeEnabled*/
												  false,		/*typeButtonEnabled*/
												  false,		/*typeTabEnabled*/
												  false,		/*typeCounterEnabled*/
												  false,		/*typeSchemasTabsEnabled*/
												  TuningFilter::Source::User,
												  TuningClientAppSettings::instance().user().m_dialogFiltersEditorSplitterPosition,
												  TuningClientAppSettings::instance().user().m_dialogFiltersEditorPropertyEditorSplitterPosition
												  );

	connect(m_tuningFilterEditor, &TuningFilterEditor::getCurrentSignalValue, this, &DialogFilterEditor::onGetCurrentSignalValue, Qt::DirectConnection);

	m_okButton = new QPushButton(tr("OK"));
	connect(m_okButton, &QPushButton::clicked, this, &DialogFilterEditor::accept);

    m_cancelButton = new QPushButton(tr("Cancel"));
    connect(m_cancelButton, &QPushButton::clicked, this, &DialogFilterEditor::reject);

    QHBoxLayout* okCancelButtonsLayout = new QHBoxLayout();
    okCancelButtonsLayout->addStretch();
	okCancelButtonsLayout->addWidget(m_okButton);
    okCancelButtonsLayout->addWidget(m_cancelButton);

    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_tuningFilterEditor);
    l->addLayout(okCancelButtonsLayout);

	if (TuningClientAppSettings::instance().user().m_dialogFiltersEditorPos.x() != -1 && TuningClientAppSettings::instance().user().m_dialogFiltersEditorPos.y() != -1)
    {
		move(TuningClientAppSettings::instance().user().m_dialogFiltersEditorPos);
		restoreGeometry(TuningClientAppSettings::instance().user().m_dialogFiltersEditorGeometry);
    }
    else
    {
        resize(1024, 768);
    }
}

DialogFilterEditor::~DialogFilterEditor()
{
	TuningClientAppSettings::instance().user().m_dialogFiltersEditorPos = pos();
	TuningClientAppSettings::instance().user().m_dialogFiltersEditorGeometry = saveGeometry();

	m_tuningFilterEditor->saveUserInterfaceSettings(&TuningClientAppSettings::instance().user().m_dialogFiltersEditorSplitterPosition, &TuningClientAppSettings::instance().user().m_dialogFiltersEditorPropertyEditorSplitterPosition);

}

void DialogFilterEditor::onGetCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok)
{
    *ok = true;

	TuningSignalState tss = m_tuningSignalManager.queuedState(appSignalHash, ok);

	if (*ok == false)
	{
		return;
	}

	if (tss.valid() == false)
	{
        *ok = false;
        return;
	}

	*value = tss.value();
}
