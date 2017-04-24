#include "TuningClientFilterEditor.h"
#include "MainWindow.h"


TuningClientFilterEditor::TuningClientFilterEditor(TuningSignalManager *tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, bool showAutomatic,
							std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
							QPoint pos,
							QByteArray geometry,
							QWidget *parent):
	TuningFilterEditor(filterStorage, objects, showAutomatic, signalsTableColumnWidth, presetsTreeColumnWidth, pos, geometry, parent),
	m_tuningSignalManager(tuningSignalManager)
{
	assert(tuningSignalManager);
	assert(filterStorage);
	assert(objects);
}

double TuningClientFilterEditor::getCurrentSignalValue(Hash appSignalHash, bool &ok)
{
    ok = true;

	QMutexLocker l(&m_tuningSignalManager->m_mutex);

	if (m_tuningSignalManager->objectExists(appSignalHash) == false)
    {
        ok = false;
        return 0;
    }

	TuningSignal* baseObject = m_tuningSignalManager->objectPtrByHash(appSignalHash);

    if (baseObject == nullptr)
    {
        ok = false;
        return 0;
    }

	if (baseObject->state.valid() == false)
    {
        ok = false;
        return 0;
    }

    return baseObject->value();
}
