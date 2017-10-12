#include "DialogFilterEditor.h"
#include "MainWindow.h"


DialogFilterEditor::DialogFilterEditor(TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects,
                                                   QWidget* parent):
    QDialog(parent),
	m_tuningSignalManager(tuningSignalManager)
{
	assert(tuningSignalManager);
	assert(filterStorage);
	assert(objects);

    m_tuningFilterEditor = new TuningFilterEditor(filterStorage, objects,
												  false, true,
												  theSettings.m_tuningFiltersPropertyEditorSplitterPos,
												  theSettings.m_tuningFiltersDialogChooseSignalGeometry);

	//connect(m_tuningFilterEditor, &TuningFilterEditor::getCurrentSignalValue, this, &DialogFilterEditor::onGetCurrentSignalValue, Qt::DirectConnection);

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

void DialogFilterEditor::onGetCurrentSignalValue(Hash appSignalHash, float* value, bool* ok)
{
    *ok = true;

	QMutexLocker lsignal(&m_tuningSignalManager->m_signalsMutex);

	if (m_tuningSignalManager->signalExists(appSignalHash) == false)
	{
        *ok = false;
        return;
	}

	lsignal.unlock();

	QMutexLocker lstate(&m_tuningSignalManager->m_statesMutex);

    TuningSignalState state = m_tuningSignalManager->stateByHash(appSignalHash);

	if (state.valid() == false)
	{
        *ok = false;
        return;
	}

    *value = state.value();

}
