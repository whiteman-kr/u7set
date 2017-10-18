#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "TuningFilter.h"
#include "TuningSignalState.h"
#include "TuningModel.h"
#include "../lib/PropertyEditor.h"



class DialogChooseTuningSignals : public QDialog
{
	Q_OBJECT

public:

	DialogChooseTuningSignals(const TuningSignalStorage* signalStorage, std::shared_ptr<TuningFilter> filter, bool setCurrentEnabled, QWidget* parent);


	enum class FilterType
	{
		All,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption
	};

	enum class SignalType
	{
		All,
		Analog,
		Discrete
	};


private:

	void fillBaseSignalsList();

	void fillFilterValuesList();

	void accept() override;

	void setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterValue& value);

private:

	const TuningSignalStorage* m_signalStorage = nullptr;

	std::shared_ptr<TuningFilter> m_filter;

	std::vector<TuningFilterValue> m_filterValues;

	// Left side

	TuningModel* m_baseModel = nullptr;
	QTableView* m_baseSignalsTable = nullptr;

	QComboBox* m_baseSignalTypeCombo = nullptr;
	QComboBox* m_baseFilterTypeCombo = nullptr;
	QLineEdit* m_baseFilterText = nullptr;
	QPushButton* m_baseApplyFilter = nullptr;

	int m_sortColumn = 0;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	// Middle

	QPushButton* m_addValue = nullptr;
	QPushButton* m_removeValue = nullptr;

	// Right side

	QTreeWidget* m_filterValuesTree = nullptr;

	QPushButton* m_moveUp = nullptr;
	QPushButton* m_moveDown = nullptr;

	QPushButton* m_setValue = nullptr;
	QPushButton* m_setCurrent = nullptr;

	QAction* m_moveUpAction = nullptr;
	QAction* m_moveDownAction = nullptr;

	QAction* m_setValueAction = nullptr;
	QAction* m_setCurrentAction = nullptr;

	//

	QPushButton* m_buttonOk = nullptr;
	QPushButton* m_buttonCancel = nullptr;

private slots:

	void baseSortIndicatorChanged(int column, Qt::SortOrder order);

	void on_m_baseApplyFilter_clicked();

	void on_m_baseFilterTypeCombo_currentIndexChanged(int index);

	void on_m_baseFilterText_returnPressed();

	void on_m_baseSignalTypeCombo_currentIndexChanged(int index);

	void on_m_baseSignalsTable_doubleClicked(const QModelIndex& index);

	void on_m_filterValuesTree_doubleClicked(const QModelIndex& index);

	void on_m_moveUp_clicked();

	void on_m_moveDown_clicked();

	void on_m_add_clicked();

	void on_m_remove_clicked();

	void on_m_setValue_clicked();

	void on_m_setCurrent_clicked();
};

class TuningFilterEditor : public QWidget
{
	Q_OBJECT

public:

	explicit TuningFilterEditor(TuningFilterStorage* filterStorage, const TuningSignalStorage* objects,
								bool readOnly,
								bool setCurrentEnabled,
								TuningFilter::Source source,
								int propertyEditorSplitterPos,
								const QByteArray& dialogChooseSignalGeometry);

	~TuningFilterEditor();

	 void saveUserInterfaceSettings(int* propertyEditorSplitterPos, QByteArray* dialogChooseSignalGeometry);


signals:

	//void getCurrentSignalValue(Hash appSignalHash, float* value, bool* ok);

private slots:

	void on_m_addPreset_clicked();

	void on_m_removePreset_clicked();

	void on_m_copyPreset_clicked();

	void on_m_pastePreset_clicked();

	void on_m_presetsTree_itemSelectionChanged();

	void on_m_presetsTree_contextMenu(const QPoint& pos);

	void presetPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void on_m_presetsSignals_clicked();
private:

	void initUserInterface();

	void addChildTreeObjects(const std::shared_ptr<TuningFilter>& filter, QTreeWidgetItem* parent);

	void setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter);

	std::shared_ptr<TuningFilter> selectedFilter(QTreeWidgetItem** item);


private:

	// User interface
	//

	QComboBox* m_filterTypeCombo = nullptr;
	QLineEdit* m_filterText = nullptr;
	QPushButton* m_applyFilter = nullptr;

	//

	QTreeWidget* m_presetsTree = nullptr;
	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;

	//

	QPushButton* m_addPreset = nullptr;
	QPushButton* m_removePreset = nullptr;

	QPushButton* m_copyPreset = nullptr;
	QPushButton* m_pastePreset = nullptr;

	QPushButton* m_presetSignals = nullptr;

	QAction* m_addPresetAction = nullptr;
	QAction* m_removePresetAction = nullptr;

	QAction* m_copyPresetAction = nullptr;
	QAction* m_pastePresetAction = nullptr;

	QMenu* m_presetsTreeContextMenu = nullptr;

	// Dialog Data
	//

	bool m_modified = false;

	TuningFilterStorage* m_filterStorage = nullptr;

	const TuningSignalStorage* m_signalStorage = nullptr;

private:

    // Apperance
    //
	QByteArray m_dialogChooseSignalGeometry;

    int m_propertyEditorSplitterPos = -1;
	bool m_readOnly = false;
	bool m_setCurrentEnabled = false;
	TuningFilter::Source m_source = TuningFilter::Source::User;
};

#endif // DIALOGPRESETEDITOR_H
