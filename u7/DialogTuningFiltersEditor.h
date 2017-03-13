#ifndef DIALOGTUNINGFILTERSEDITOR_H
#define DIALOGTUNINGFILTERSEDITOR_H

#include "../lib/TuningFilterEditor.h"

class DialogTuningFiltersEditor : public TuningFilterEditor
{
public:
    DialogTuningFiltersEditor(TuningFilterStorage* filterStorage, const TuningObjectStorage* objects, bool showAutomatic,
                              std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                              QPoint pos,
                              QByteArray geometry,
                              QWidget *parent);
};

extern DialogTuningFiltersEditor* theDialogTuningFiltersEditor;

#endif // DIALOGTUNINGFILTERSEDITOR_H
