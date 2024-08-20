#pragma once

#include <TuningLib/TuningUiItem.h>

namespace ExtWidgets
{
	class PropertyEditor;
};

class TuningUiEditor : public QWidget
{
	Q_OBJECT

public:
	TuningUiEditor(TuningLib::TuningUiStorage& storage,
				   bool readOnly,
				   bool typeTreeEnabled,
				   bool typeButtonEnabled,
				   bool typeTabEnabled,
				   bool typeCounterEnabled,
				   bool typeSchemasTabsEnabled);

	~TuningUiEditor();

	bool readOnly() const;
	void setReadOnly(bool value);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void onAdd();
	void onRemove();
	void onMoveUp();
	void onMoveDown();
	void onCopy();
	void onPaste();
	void onItemSelectionChanged();
	void onContextMenu(const QPoint& pos);
	void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

private:
	void initUi();

	void addItem(TuningLib::TuningUiItem::InterfaceType uiType);
	void addChildTreeItems(TuningLib::TuningUiItem* uiItem, QTreeWidgetItem* parent);
	void setItemText(QTreeWidgetItem* treeItem, TuningLib::TuningUiItem* uiItem);
	void moveItems(int direction);

private:
	// User interface
	//

	QComboBox* m_filterTypeCombo = nullptr;
	QLineEdit* m_filterText = nullptr;
	QPushButton* m_applyFilter = nullptr;

	//
	QSplitter* m_hSplitter = nullptr;

	QTreeWidget* m_itemsTree = nullptr;
	ExtWidgets::PropertyEditor* m_propertyEditor = nullptr;

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

	QMenu* m_itemsTreeContextMenu = nullptr;

	// Dialog Data
	//

	bool m_modified = false;

	TuningLib::TuningUiStorage& m_uiStorage;

private:
	bool m_readOnly = false;

	bool m_typeButtonEnabled = false;
	bool m_typeTabEnabled = false;
	bool m_typeTreeEnabled = false;
	bool m_typeCounterEnabled = false;
	bool m_typeSchemasTabsEnabled = false;
};

