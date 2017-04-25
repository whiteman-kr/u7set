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

	QMutexLocker lsignal(&m_tuningSignalManager->m_signalsMutex);

	if (m_tuningSignalManager->signalExists(appSignalHash) == false)
    {
        ok = false;
        return 0;
    }

	lsignal.unlock();

	QMutexLocker lstate(&m_tuningSignalManager->m_statesMutex);

	TuningSignalState& state = m_tuningSignalManager->stateByHash(appSignalHash);

	if (state.valid() == false)
    {
        ok = false;
        return 0;
    }

	return state.value();
}
