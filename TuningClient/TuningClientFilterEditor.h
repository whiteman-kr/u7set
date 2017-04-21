#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilterEditor.h"

class TuningClientFilterEditor : public TuningFilterEditor
{
public:

	explicit TuningClientFilterEditor(TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, bool showAutomatic,
                                std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                                QPoint pos,
                                QByteArray geometry,
                                QWidget *parent);

protected:

    virtual double getCurrentSignalValue(Hash appSignalHash, bool &ok) override;

private:
	TuningSignalManager* m_tuningSignalManager = nullptr;
};

#endif // TUNINGCLIENTFILTEREDITOR_H
