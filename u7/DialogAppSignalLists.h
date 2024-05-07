#pragma once

namespace AppSignalLists
{
	class AppSignalList;
	class AppSignalListWidget;
} // namespace AppSignalLists

class IdePropertyEditor;

namespace Builder
{
	class AppSignalListsProvider;
	class AppSignalListStorage;
}

class DbController;

class DialogAppSignalLists : public QDialog
{
public:
	static void showDialog(DbController* db, QWidget* parent);

public:
	DialogAppSignalLists(DbController* db, QWidget* parent);
	~DialogAppSignalLists();

	void setFilter(QString filter);

private slots:
	void onMaskReturn();
	void onMaskApply();

	void onSortIndicatorChanged(int column, Qt::SortOrder order);

	void onItemSelectionChanged();
	void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);
	void onSignalsChanged();

	void onAdd();
	void onRemove();
	void onCopy();
	void onPaste();
	void onCheckOut();
	void onCheckIn();
	void onUndo();
	void onRefresh();

	void onCopyShortcut();
	void onPasteShortcut();
	void onRemoveShortcut();

	void onCustomContextMenuRequested(const QPoint& pos);

private:
	bool addList(std::shared_ptr<AppSignalLists::AppSignalList> list);
	bool pasteList(std::shared_ptr<AppSignalLists::AppSignalList> list);

	void fillAppSignalLists();
	void setPropertyEditorObjects();
	bool continueWithDuplicateIds();
	void updateTreeItemText(QTreeWidgetItem* item);
	void updateButtonsEnableState();

protected:
	virtual void closeEvent(QCloseEvent* e);
	virtual void reject();

private:
	QLineEdit* m_mask = nullptr;
	QPushButton* m_maskApply = nullptr;

	QPushButton* m_btnAdd = nullptr;
	QPushButton* m_btnRemove = nullptr;
	QPushButton* m_btnCheckOut = nullptr;
	QPushButton* m_btnCheckIn = nullptr;
	QPushButton* m_btnUndo = nullptr;
	QPushButton* m_btnRefresh = nullptr;
	QPushButton* m_btnClose = nullptr;

	QTreeWidget* m_listsTree = nullptr;

	IdePropertyEditor* m_listPropertyEditor = nullptr;
	AppSignalLists::AppSignalListWidget* m_signalListWidget = nullptr;

	QSplitter* m_splitter = nullptr;

	QCompleter* m_completer = nullptr;

	QStringList m_masks;

	DbController* m_db = nullptr;

	std::unique_ptr<Builder::AppSignalListsProvider> m_signalProvider;
	std::unique_ptr<Builder::AppSignalListStorage> m_lists;

	QMenu* m_popupMenu = nullptr;
	QAction* m_addAction = nullptr;
	QAction* m_removeAction = nullptr;
	QAction* m_copyAction = nullptr;
	QAction* m_pasteAction = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoAction = nullptr;
	QAction* m_refreshAction = nullptr;


	inline static DialogAppSignalLists* s_instance = nullptr;
};
