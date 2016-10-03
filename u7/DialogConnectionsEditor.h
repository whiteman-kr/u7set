#ifndef DIALOGCONNECTIONSEDITOR_H
#define DIALOGCONNECTIONSEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include <QCompleter>
#include "../lib/DbController.h"
#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "Connection.h"

class DialogConnectionsPropertyEditor : public PropertyEditorDialog
{
public:
	DialogConnectionsPropertyEditor(std::shared_ptr<PropertyObject> object, QWidget *parent, Hardware::ConnectionStorage* connections, bool readOnly);
    ~DialogConnectionsPropertyEditor();

private:
    virtual bool onPropertiesChanged(std::shared_ptr<PropertyObject> object);
    virtual void closeEvent(QCloseEvent * e);
    virtual void done(int r);

    void saveSettings();

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
	void updateButtons();
	bool continueWithDuplicateCaptions();

	void setConnectionText(QTreeWidgetItem* item, Hardware::Connection* connection);

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
	void reject();

	void on_m_list_itemSelectionChanged();

	void on_m_Apply_clicked();
	void sortIndicatorChanged(int column, Qt::SortOrder order);

	void on_m_mask_returnPressed();

	void on_m_Export_clicked();

	void on_m_search_clicked();

private:
    Ui::DialogConnectionsEditor *ui;

    bool m_modified = false;

	QCompleter* m_completer = nullptr;

	QStringList m_masks;

    DbController* db();
    DbController* m_dbController;

    Hardware::ConnectionStorage connections;

	bool m_checkedOut = false;
};

extern DialogConnectionsEditor* theDialogConnectionsEditor;

#endif // DIALOGCONNECTIONSEDITOR_H
