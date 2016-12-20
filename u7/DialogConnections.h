#ifndef DIALOGCONNECTIONS_H
#define DIALOGCONNECTIONS_H

#include "Stable.h"
#include <QItemDelegate>
#include <QCompleter>
#include "../lib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "Connection.h"

class DialogConnections : public QDialog
{
public:
    DialogConnections(DbController *pDbController, QWidget *parent);
    ~DialogConnections();

private slots:
    void onMaskReturn();
    void onMaskApply();

    void onSortIndicatorChanged(int column, Qt::SortOrder order);

    void onItemSelectionChanged();
    void onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

    void onAdd();
    void onRemove();
    void onCheckOut();
    void onCheckIn();
    void onUndo();
    void onReport();
    void onRefresh();

    void onCustomContextMenuRequested(const QPoint &pos);

private:
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

    DbController *m_dbController = nullptr;

    Hardware::ConnectionStorage *m_connections;

    QMenu* m_popupMenu = nullptr;
    QAction* m_addAction = nullptr;
    QAction* m_removeAction = nullptr;
    QAction* m_checkOutAction = nullptr;
    QAction* m_checkInAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_refreshAction = nullptr;


};

extern DialogConnections* theDialogConnections;

#endif // DIALOGCONNECTIONS_H
