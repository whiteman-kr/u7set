#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "TuningFilter.h"
#include "ObjectManager.h"
#include "TuningObject.h"
#include "TuningPage.h"

namespace Ui {
class DialogPresetEditor;


}


class DialogPresetEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogPresetEditor(TuningFilterStorage* filterStorage, QWidget *parent = 0);
	~DialogPresetEditor();

private slots:
	void on_m_addPreset_clicked();

	void on_m_editPreset_clicked();

	void on_m_removePreset_clicked();

	void on_m_moveUp_clicked();

	void on_m_moveDown_clicked();

	void on_m_add_clicked();

	void on_m_remove_clicked();

	void on_m_presetsTree_doubleClicked(const QModelIndex &index);

	void on_m_signalTypeCombo_currentIndexChanged(int index);

	void on_m_presetsTree_itemSelectionChanged();

	void on_m_setValue_clicked();

	void on_m_applyMask_clicked();

	void on_m_signalsTable_doubleClicked(const QModelIndex &index);

	void slot_signalsUpdated();

	void sortIndicatorChanged(int column, Qt::SortOrder order);
private:

	enum class TreeItemType
	{
		Filter,
		Signal
	};

	enum class MaskType
	{
		AppSignalID,
		CustomAppSignalID,
		EquipmentID
	};

	enum class SignalType
	{
		All,
		Analog,
		Discrete
	};

	bool isFilter(QTreeWidgetItem* item);
	bool isSignal(QTreeWidgetItem* item);

	void addChildTreeObjects(const std::shared_ptr<TuningFilter> &filter, QTreeWidgetItem* parent);

	void setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter);
	void setSignalItemText(QTreeWidgetItem* item, const TuningFilterValue& value);

	void fillObjectsList();

	std::shared_ptr<TuningFilter> selectedFilter(QTreeWidgetItem** item);

	void getSelectedCount(int& selectedPresets, int& selectedSignals);


private:
	Ui::DialogPresetEditor *ui;

	TuningItemModel *m_model = nullptr;

	bool m_modified = false;

	TuningFilterStorage* m_filterStorage;

	int m_sortColumn = 0;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // DIALOGPRESETEDITOR_H
