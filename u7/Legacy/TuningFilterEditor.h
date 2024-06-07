#pragma once

#include "../../lib/Tuning/TuningFilter.h"
#include <UiLib/PropertyEditor.h>


class ViewTuningSignalsWidget : public QWidget
{
	Q_OBJECT

public:

	ViewTuningSignalsWidget(ClientLib::TuningSignalManager& signalManager, bool requestValuesEnabled, QWidget* parent);

	enum class BaseFilterType
	{
		All,
		AppSignalID,
		CustomAppSignalID,
		EquipmentID,
		Caption,
		Tag
	};

	enum class ValueFilterType
	{
		All,
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
	bool readOnly() const;
	void setReadOnly(bool value);

	void setFilter(std::shared_ptr<TuningFilter> selectedFilter);

private:
	void fillFilterValuesTree();
	void setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterSignal& value);

private:

	ClientLib::TuningSignalManager& m_signalManager;

	std::shared_ptr<TuningFilter> m_filter;

	// Right side

	QTreeWidget* m_filterValuesTree = nullptr;
	QPushButton* m_exportValues = nullptr;

	//
	bool m_readOnly = false;

private slots:
	void on_m_exportValues_clicked();
};

class TuningFilterEditor : public QWidget
{
	Q_OBJECT

public:

	explicit TuningFilterEditor(TuningFilterStorage& filterStorage,
								ClientLib::TuningSignalManager& signalManager,
								bool readOnly,
								bool requestValuesEnabled,
								bool typeTreeEnabled,
								bool typeButtonEnabled,
								bool typeTabEnabled,
								bool typeCounterEnabled,
								bool typeSchemasTabsEnabled,
								TuningFilter::Source source,
								QByteArray mainSplitterState,
								int propertyEditorSplitterPos);

	~TuningFilterEditor();

	bool readOnly() const;
	void setReadOnly(bool value);

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

	ViewTuningSignalsWidget* m_viewTuningSignalsWidget = nullptr;

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

	TuningFilterStorage& m_filterStorage;

	ClientLib::TuningSignalManager& m_signalManager;

private:

    // Apperance
    //
	QByteArray m_dialogChooseSignalGeometry;

	bool m_readOnly = false;
	bool m_requestValuesEnabled = false;

	bool m_typeButtonEnabled = false;
	bool m_typeTabEnabled = false;
	bool m_typeTreeEnabled = false;
	bool m_typeCounterEnabled = false;
	bool m_typeSchemasTabsEnabled = false;

	TuningFilter::Source m_source = TuningFilter::Source::User;
};

//
// IdeTuningFiltersEditor
//
class IdeTuningFiltersEditor : public ExtWidgets::PropertyTextEditor
{
public:
	explicit IdeTuningFiltersEditor(DbController* dbController, QWidget* parent);
    virtual ~IdeTuningFiltersEditor();

	QString text() const override;
	void setText(const QString& text) override;

	bool readOnly() const override;
    void setReadOnly(bool value) override;

	bool externalOkCancelButtons() const override;

private:
	bool isModified() const override;

private:
    TuningFilterEditor* m_tuningFilterEditor = nullptr;

	ILogFileStub logFileStub;
    ClientLib::TuningSignalManager m_signals;
	TuningFilterStorage m_filters;

	DbController* m_dbController = nullptr;
};