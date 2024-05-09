#pragma once

#include "../../Builder/DiagSignalTypesStorage.h"

namespace ExtWidgets
{
	class PropertyEditor;
}

class DialogDiagSignalTypes : public QDialog
{
	Q_OBJECT

private:
	explicit DialogDiagSignalTypes(DbController* db, QWidget* parent);
	virtual ~DialogDiagSignalTypes();

public:
	static void showDialog(DbController* db, QWidget* parent);

private slots:
	void onItemSelectionChanged();
	void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void onAdd();
	void onRemove();
	void onCopy();
	void onPaste();
	void onCheckOut();
	void onCheckIn();
	void onUndo();
	void onRefresh();
	void onExport();
	void onImport();

	void onCopyShortcut();
	void onPasteShortcut();
	void onRemoveShortcut();

	void onCustomContextMenuRequested(const QPoint& pos);

private:
	bool addDiagSignalType(std::shared_ptr<Hardware::DiagSignalTypeObject> dst);
	bool pasteDiagSignalType(std::shared_ptr<Hardware::DiagSignalTypeObject> dst);

	QString signalTypeIdFromItem(const QUuid& uuid) const;

	void fillDiagSignalTypesList();
	void setPropertyEditorObjects();
	bool continueWithDuplicateCaptions();
	void updateTreeItemText(QTreeWidgetItem* item);
	void updateButtonsEnableState();

protected:
	virtual void closeEvent(QCloseEvent* e);
	virtual void reject();

private:
	enum class Columns
	{
		SignalTypeId,
		Action,
		UserId,
		Count
	};

	QPushButton* m_btnAdd = nullptr;
	QPushButton* m_btnRemove = nullptr;
	QPushButton* m_btnCheckOut = nullptr;
	QPushButton* m_btnCheckIn = nullptr;
	QPushButton* m_btnUndo = nullptr;
	QPushButton* m_btnRefresh = nullptr;
	QPushButton* m_btnClose = nullptr;

	QTreeWidget* m_diagSignalTypesTree = nullptr;
	ExtWidgets::PropertyEditor* m_diagSignalTypesPropertyEditor = nullptr;

	QSplitter* m_splitter = nullptr;

	QMenu* m_popupMenu = nullptr;
	QAction* m_addAction = nullptr;
	QAction* m_removeAction = nullptr;
	QAction* m_copyAction = nullptr;
	QAction* m_pasteAction = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoAction = nullptr;
	QAction* m_refreshAction = nullptr;
	QAction* m_importAction = nullptr;
	QAction* m_exportAction = nullptr;

	// --
	//
	DbController* m_db = nullptr;
	DiagSignalTypesStorage m_diagSignalTypes;

	// --
	//
	inline static DialogDiagSignalTypes* s_instance = nullptr;
};
