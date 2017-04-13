#include "ClientFilterEditor.h"
#include "MainWindow.h"


TuningClientFilterEditor::TuningClientFilterEditor(TuningObjectManager *tuningObjectManager, TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
							std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
							QPoint pos,
							QByteArray geometry,
							QWidget *parent):
	TuningFilterEditor(filterStorage, objects, showAutomatic, signalsTableColumnWidth, presetsTreeColumnWidth, pos, geometry, parent),
	m_tuningObjectManager(tuningObjectManager)
{
	assert(tuningObjectManager);
	assert(filterStorage);
	assert(objects);
}

double TuningClientFilterEditor::getCurrentSignalValue(Hash appSignalHash, bool &ok)
{
    ok = true;

	QMutexLocker l(&m_tuningObjectManager->m_mutex);

	if (m_tuningObjectManager->objectExists(appSignalHash) == false)
    {
        ok = false;
        return 0;
    }

	TuningObject* baseObject = m_tuningObjectManager->objectPtrByHash(appSignalHash);

    if (baseObject == nullptr)
    {
        ok = false;
        return 0;
    }

    if (baseObject->valid() == false)
    {
        ok = false;
        return 0;
    }

    return baseObject->value();
}
