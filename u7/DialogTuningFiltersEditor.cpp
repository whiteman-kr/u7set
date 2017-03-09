#include "DialogTuningFiltersEditor.h"

DialogTuningFiltersEditor* theDialogTuningFiltersEditor = nullptr;

DialogTuningFiltersEditor::DialogTuningFiltersEditor(TuningFilterStorage *filterStorage, const TuningObjectStorage *objects, bool showAutomatic, std::vector<int> &signalsTableColumnWidth, std::vector<int> &presetsTreeColumnWidth, QPoint pos, QByteArray geometry, QWidget *parent)
    :TuningFilterEditor(filterStorage, objects, showAutomatic, signalsTableColumnWidth, presetsTreeColumnWidth, pos, geometry, parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

}
