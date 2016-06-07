#ifndef DIALOGSUBSYSTEMLISTEDITOR_H
#define DIALOGSUBSYSTEMLISTEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include "../lib/DbController.h"
#include "Subsystem.h"

namespace Ui {
class DialogSubsystemListEditor;
}

class EditorDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	EditorDelegate(QObject *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

class DialogSubsystemListEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSubsystemListEditor(DbController* pDbController, QWidget *parent = 0);
	~DialogSubsystemListEditor();

private:
	bool askForSaveChanged();
	bool saveChanges();

protected:
	virtual void closeEvent(QCloseEvent* e);

private slots:
	void on_m_add_clicked();
	void on_m_remove_clicked();
	void on_buttonOk_clicked();
	void on_buttonCancel_clicked();
	void on_m_list_itemChanged(QTreeWidgetItem *item, int column);

private:
	Ui::DialogSubsystemListEditor *ui;

	bool m_modified = false;
	EditorDelegate* m_editorDelegate = nullptr;


	DbController* db();
	DbController* m_dbController;
};

#endif // DIALOGSUBSYSTEMLISTEDITOR_H
