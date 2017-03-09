#ifndef TUNINGCLIENTFILTEREDITOR_H
#define TUNINGCLIENTFILTEREDITOR_H

#include "../lib/TuningFilterEditor.h"

class TuningClientFilterEditor : public TuningFilterEditor
{
public:

    explicit TuningClientFilterEditor(TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
                                std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                                QPoint pos,
                                QByteArray geometry,
                                QWidget *parent);

protected:

    virtual double getCurrentSignalValue(Hash appSignalHash, bool &ok) override;
};

#endif // TUNINGCLIENTFILTEREDITOR_H
