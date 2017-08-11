#include "DialogTuningFiltersEditor.h"

DialogTuningFiltersEditor* theDialogTuningFiltersEditor = nullptr;

DialogTuningFiltersEditor::DialogTuningFiltersEditor(TuningFilterStorage *filterStorage, const TuningSignalStorage *objects, std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth, QPoint pos, QByteArray geometry, QWidget *parent)
	:TuningFilterEditor(filterStorage, objects, signalsTableColumnWidth, presetsTreeColumnWidth, pos, geometry, parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

}
