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
    void onMaskReturnPressed();
    void onMaskApplyClicked();
    void sortIndicatorChanged(int column, Qt::SortOrder order);
    void onConnectionItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);


private:
    void fillConnectionsList();
    bool continueWithDuplicateCaptions();
    void setConnectionText(QTreeWidgetItem* item, Hardware::Connection* connection);

private:

    QLineEdit* m_mask = nullptr;
    QPushButton* m_maskApply = nullptr;

    QPushButton* m_btnAdd = nullptr;
    QPushButton* m_btnRemove = nullptr;
    QPushButton* m_btnCheckOut = nullptr;
    QPushButton* m_btnCheckIn = nullptr;
    QPushButton* m_btnUndo = nullptr;
    QPushButton* m_btnExport = nullptr;
    QPushButton* m_btnClose = nullptr;

    QTreeWidget* m_connectionsTree = nullptr;
    ExtWidgets::PropertyEditor* m_connectionPropertyEditor = nullptr;

    QSplitter* m_splitter = nullptr;

    QCompleter* m_completer = nullptr;

    QStringList m_masks;

    DbController *m_dbController = nullptr;

    Hardware::ConnectionStorage connections;

};

extern DialogConnections* theDialogConnections;

#endif // DIALOGCONNECTIONS_H
