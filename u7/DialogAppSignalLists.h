#pragma once

#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "../AppSignalLib/ISignalManager.h"
#include "AppSignalListStorage.h"

class AppSignalSetProvider;

class AppSignalListsProvider : public ISignalManager
{
public:
	AppSignalListsProvider() = delete;
	AppSignalListsProvider(AppSignalSetProvider* signalSetProvider);

	virtual int signalsCount() const override;
    virtual std::vector<Hash> signalHashes() const override;
	virtual std::vector<AppSignalParam> signalList() const override;

	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;
	virtual bool signalsExist(const QStringList& signalIds) const override;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

private:
    AppSignalSetProvider* m_signalSetProvider = nullptr;
};

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

	void onCustomContextMenuRequested(const QPoint &pos);

private:
	bool addList(std::shared_ptr<AppSignalLists::AppSignalList> list);
	bool pasteList(std::shared_ptr<AppSignalLists::AppSignalList> list);

    void fillAppSignalLists();
    void setPropertyEditorObjects();
    bool continueWithDuplicateIds();
    void updateTreeItemText(QTreeWidgetItem *item);
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
    
    ExtWidgets::PropertyEditor* m_listPropertyEditor = nullptr;
    AppSignalLists::AppSignalListWidget* m_signalListWidget = nullptr;

    QSplitter* m_splitter = nullptr;

    QCompleter* m_completer = nullptr;

    QStringList m_masks;

	DbController* m_db = nullptr;

	AppSignalListStorage m_lists;

    AppSignalListsProvider m_signalProvider;

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

