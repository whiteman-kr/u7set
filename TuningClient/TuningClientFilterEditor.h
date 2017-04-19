#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/Tuning/TuningObjectManager.h"
#include "../lib/Tuning/TuningFilterEditor.h"

class TuningClientFilterEditor : public TuningFilterEditor
{
public:

	explicit TuningClientFilterEditor(TuningObjectManager* tuningObjectManager, TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
                                std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                                QPoint pos,
                                QByteArray geometry,
                                QWidget *parent);

protected:

    virtual double getCurrentSignalValue(Hash appSignalHash, bool &ok) override;

private:
	TuningObjectManager* m_tuningObjectManager = nullptr;
};

#endif // TUNINGCLIENTFILTEREDITOR_H
