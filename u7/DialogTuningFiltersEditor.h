#ifndef DIALOGTUNINGFILTERSEDITOR_H
#define DIALOGTUNINGFILTERSEDITOR_H

#include "../lib/Tuning/TuningFilterEditor.h"

class DialogTuningFiltersEditor : public TuningFilterEditor
{
public:
	DialogTuningFiltersEditor(TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, bool showAutomatic,
                              std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth,
                              QPoint pos,
                              QByteArray geometry,
                              QWidget *parent);
};

extern DialogTuningFiltersEditor* theDialogTuningFiltersEditor;

#endif // DIALOGTUNINGFILTERSEDITOR_H
