#include "TuningClientFilterEditor.h"
#include "MainWindow.h"


TuningClientFilterEditor::TuningClientFilterEditor(TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
                            std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                            QPoint pos,
                            QByteArray geometry,
                            QWidget *parent):
    TuningFilterEditor(filterStorage, objects, showAutomatic, signalsTableColumnWidth, presetsTreeColumnWidth, pos, geometry, parent)
{
}

double TuningClientFilterEditor::getCurrentSignalValue(Hash appSignalHash, bool &ok)
{
    ok = true;

    QMutexLocker l(&theObjectManager->m_mutex);

    if (theObjectManager->objectExists(appSignalHash) == false)
    {
        ok = false;
        return 0;
    }

    TuningObject* baseObject = theObjectManager->objectPtrByHash(appSignalHash);

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
