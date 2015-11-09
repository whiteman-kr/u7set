#ifndef DIALOGCONNECTIONSEDITOR_H
#define DIALOGCONNECTIONSEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include "../include/DbController.h"
#include "../include/PropertyEditor.h"
#include "Connection.h"

namespace Ui {
class DialogConnectionsEditor;
}

class DialogConnectionsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConnectionsEditor(DbController* pDbController, QWidget *parent = 0);
    ~DialogConnectionsEditor();

private:

    void fillConnectionsList();
    bool askForSaveChanged();
    bool checkUniqueConnections();
    bool saveChanges();

protected:
    virtual void closeEvent(QCloseEvent* e);

private slots:
    void on_m_Add_clicked();
    void on_m_Remove_clicked();
    void on_m_OK_clicked();
    void on_m_Cancel_clicked();

    void on_m_Edit_clicked();

    void on_m_list_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogConnectionsEditor *ui;

    bool m_modified = false;

    DbController* db();
    DbController* m_dbController;

    Hardware::ConnectionStorage m_connections;
};

#endif // DIALOGCONNECTIONSEDITOR_H
