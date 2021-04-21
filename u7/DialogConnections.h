#ifndef DIALOGCONNECTIONS_H
#define DIALOGCONNECTIONS_H

#include "../DbLib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "../lib/Connection.h"

class DialogConnections : public QDialog
{
public:
	DialogConnections(DbController* db, QWidget* parent);
    ~DialogConnections();

	void setFilter(QString filter);
	bool addConnection(QString port1Id, QString port2Id);

private slots:
    void onMaskReturn();
    void onMaskApply();
	void onMaskReset();

    void onSortIndicatorChanged(int column, Qt::SortOrder order);

    void onItemSelectionChanged();
    void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

    void onAdd();
    void onRemove();
	void onCopy();
	void onPaste();
    void onCheckOut();
    void onCheckIn();
    void onUndo();
    void onReport();
    void onRefresh();

	void onCopyShortcut();
	void onPasteShortcut();
	void onRemoveShortcut();

	void onCustomContextMenuRequested(const QPoint &pos);

private:
	bool addConnection(std::shared_ptr<Hardware::Connection> connection);
	bool pasteConnection(std::shared_ptr<Hardware::Connection> connection);

    void fillConnectionsList();
    void setPropertyEditorObjects();
    bool continueWithDuplicateCaptions();
    void updateTreeItemText(QTreeWidgetItem *item);
    void updateButtonsEnableState();

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual void reject();

private:

    QLineEdit* m_mask = nullptr;
    QPushButton* m_maskApply = nullptr;
	QPushButton* m_maskReset = nullptr;

    QPushButton* m_btnAdd = nullptr;
    QPushButton* m_btnRemove = nullptr;
    QPushButton* m_btnCheckOut = nullptr;
    QPushButton* m_btnCheckIn = nullptr;
    QPushButton* m_btnUndo = nullptr;
    QPushButton* m_btnRefresh = nullptr;
    QPushButton* m_btnReport = nullptr;
    QPushButton* m_btnClose = nullptr;

    QTreeWidget* m_connectionsTree = nullptr;
    ExtWidgets::PropertyEditor* m_connectionPropertyEditor = nullptr;

    QSplitter* m_splitter = nullptr;

    QCompleter* m_completer = nullptr;

    QStringList m_masks;

	DbController* m_db = nullptr;

	Hardware::ConnectionStorage m_connections;

    QMenu* m_popupMenu = nullptr;
    QAction* m_addAction = nullptr;
    QAction* m_removeAction = nullptr;
	QAction* m_copyAction = nullptr;
	QAction* m_pasteAction = nullptr;
    QAction* m_checkOutAction = nullptr;
    QAction* m_checkInAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_refreshAction = nullptr;
};

extern DialogConnections* theDialogConnections;

#endif // DIALOGCONNECTIONS_H
