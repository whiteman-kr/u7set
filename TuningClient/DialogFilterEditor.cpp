#include "DialogFilterEditor.h"
#include "MainWindow.h"


DialogFilterEditor::DialogFilterEditor(TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, TuningFilterStorage* filterStorage,
                                                   QWidget* parent):
    QDialog(parent),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningClientTcpClient(tuningTcpClient)
{
	assert(tuningSignalManager);
	assert(filterStorage);

	m_tuningFilterEditor = new TuningFilterEditor(filterStorage, tuningSignalManager,
												  false, true, TuningFilter::Source::User,
												  theSettings.m_tuningFiltersPropertyEditorSplitterPos,
												  theSettings.m_tuningFiltersDialogChooseSignalGeometry);

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

    if (theSettings.m_presetEditorPos.x() != -1 && theSettings.m_presetEditorPos.y() != -1)
    {
        move(theSettings.m_presetEditorPos);
        restoreGeometry(theSettings.m_presetEditorGeometry);
    }
    else
    {
        resize(1024, 768);
    }
}

DialogFilterEditor::~DialogFilterEditor()
{
    theSettings.m_presetEditorPos = pos();
    theSettings.m_presetEditorGeometry = saveGeometry();

	m_tuningFilterEditor->saveUserInterfaceSettings(&theSettings.m_tuningFiltersPropertyEditorSplitterPos, &theSettings.m_tuningFiltersDialogChooseSignalGeometry);

}

void DialogFilterEditor::onGetCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok)
{
    *ok = true;

	TuningSignalState tss = m_tuningSignalManager->state(appSignalHash, ok);

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
