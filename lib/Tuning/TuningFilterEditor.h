#ifndef DIALOGPRESETEDITOR_H
#define DIALOGPRESETEDITOR_H

#include <QDialog>
#include "TuningFilter.h"
#include "TuningSignalState.h"
#include "TuningModel.h"
#include "../lib/PropertyEditor.h"

class ChooseTuningSignalsWidget : public QWidget
{
	Q_OBJECT

public:

	ChooseTuningSignalsWidget(TuningSignalManager* signalStorage, bool requestValuesEnabled, QWidget* parent);

	enum class FilterType
	{
		All,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption,
		Zero,
		One
	};

	enum class SignalType
	{
		All,
		Analog,
		Discrete
	};

	enum class Columns
	{
		CustomAppSignalID,
		AppSignalID,
		Type,
		Caption,
		Value
	};

public:
	void setFilter(std::shared_ptr<TuningFilter> selectedFilter);

signals:
	void getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok); 	// Qt::DirectConnection!

private:

	void fillBaseSignalsList();

	void setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterSignal& value);

private:

	TuningSignalManager* m_signalManager = nullptr;

	std::shared_ptr<TuningFilter> m_filter;

	// Left side

	TuningModel* m_baseModel = nullptr;
	QTableView* m_baseSignalsTable = nullptr;

	QComboBox* m_baseSignalTypeCombo = nullptr;
	QComboBox* m_baseFilterTypeCombo = nullptr;
	QComboBox* m_baseFilterValueCombo = nullptr;
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

private slots:

	void baseSortIndicatorChanged(int column, Qt::SortOrder order);

	void on_m_baseApplyFilter_clicked();

	void on_m_baseFilterTypeCombo_currentIndexChanged(int index);

	void on_m_baseFilterValueCombo_currentIndexChanged(int index);

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

	explicit TuningFilterEditor(TuningFilterStorage* filterStorage, TuningSignalManager* signalManager,
								bool readOnly,
								bool requestValuesEnabled,
								bool typeTreeEnabled,
								bool typeButtonEnabled,
								bool typeTabEnabled,
								TuningFilter::Source source,
								QByteArray mainSplitterState,
								int propertyEditorSplitterPos);

	~TuningFilterEditor();

	 void saveUserInterfaceSettings(QByteArray* mainSplitterState, int* propertyEditorSplitterPos);

signals:

	void getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok);	// Qt::DirectConnection!

protected:

	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:

	void on_m_addPreset_clicked();

	void on_m_removePreset_clicked();

	void on_m_moveUpPreset_clicked();

	void on_m_moveDownPreset_clicked();

	void on_m_copyPreset_clicked();

	void on_m_pastePreset_clicked();

	void on_m_presetsTree_itemSelectionChanged();

	void on_m_presetsTree_contextMenu(const QPoint& pos);

	void presetPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void slot_getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok);

private:

	void initUserInterface(QByteArray mainSplitterState, int propertyEditorSplitterPos);

	void addPreset(TuningFilter::InterfaceType interfaceType);

	void addChildTreeObjects(const std::shared_ptr<TuningFilter>& filter, QTreeWidgetItem* parent);

	void setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter);

	void movePresets(int direction);


private:

	// User interface
	//

	QComboBox* m_filterTypeCombo = nullptr;
	QLineEdit* m_filterText = nullptr;
	QPushButton* m_applyFilter = nullptr;

	//
	QSplitter* m_hSplitter = nullptr;

	QTreeWidget* m_presetsTree = nullptr;
	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;

	ChooseTuningSignalsWidget* m_chooseTuningSignalsWidget = nullptr;

	//

	QPushButton* m_addPreset = nullptr;
	QPushButton* m_removePreset = nullptr;

	QPushButton* m_moveUpPreset = nullptr;
	QPushButton* m_moveDownPreset = nullptr;

	QPushButton* m_copyPreset = nullptr;
	QPushButton* m_pastePreset = nullptr;

	QAction* m_addPresetAction = nullptr;
	QAction* m_removePresetAction = nullptr;

	QAction* m_moveUpPresetAction = nullptr;
	QAction* m_moveDownPresetAction = nullptr;

	QAction* m_copyPresetAction = nullptr;
	QAction* m_pastePresetAction = nullptr;

	QMenu* m_presetsTreeContextMenu = nullptr;

	// Dialog Data
	//

	bool m_modified = false;

	TuningFilterStorage* m_filterStorage = nullptr;

	TuningSignalManager* m_signalManager = nullptr;

private:

    // Apperance
    //
	QByteArray m_dialogChooseSignalGeometry;

	bool m_readOnly = false;
	bool m_requestValuesEnabled = false;

	bool m_typeButtonEnabled = false;
	bool m_typeTabEnabled = false;
	bool m_typeTreeEnabled = false;

	TuningFilter::Source m_source = TuningFilter::Source::User;
};

#endif // DIALOGPRESETEDITOR_H
