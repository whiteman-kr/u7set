#ifndef DIALOGCONNECTIONSEDITOR_H
#define DIALOGCONNECTIONSEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include "../include/DbController.h"
#include "../include/PropertyEditor.h"
#include "../include/PropertyEditorDialog.h"
#include "Connection.h"

class DialogConnectionsPropertyEditor : public PropertyEditorDialog
{
public:
    DialogConnectionsPropertyEditor(std::shared_ptr<PropertyObject> object, QWidget *parent, Hardware::ConnectionStorage* connections);

    virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);
private:
    Hardware::ConnectionStorage *m_connections = nullptr;
};

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
    void updateButtons(bool checkOut);

protected:
    virtual void closeEvent(QCloseEvent* e);

private slots:
    void on_m_Add_clicked();
    void on_m_Remove_clicked();
    void on_m_OK_clicked();
    void on_m_Cancel_clicked();
    void on_m_Edit_clicked();
    void on_m_list_doubleClicked(const QModelIndex &index);
    void on_m_checkOut_clicked();
    void on_m_checkIn_clicked();
    void on_m_Undo_clicked();
private:
    Ui::DialogConnectionsEditor *ui;

    bool m_modified = false;

    DbController* db();
    DbController* m_dbController;

    Hardware::ConnectionStorage connections;
};

#endif // DIALOGCONNECTIONSEDITOR_H
