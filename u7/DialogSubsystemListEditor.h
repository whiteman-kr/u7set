#ifndef DIALOGSUBSYSTEMLISTEDITOR_H
#define DIALOGSUBSYSTEMLISTEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include "../include/DbController.h"
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
};

class DialogSubsystemListEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSubsystemListEditor(DbController* pDbController, QWidget *parent = 0);
	~DialogSubsystemListEditor();

private:
	void fillList();

private slots:
	void on_m_add_clicked();
	void on_m_remove_clicked();
	void on_DialogSubsystemListEditor_accepted();

private:
	Ui::DialogSubsystemListEditor *ui;


	DbController* db();
	DbController* m_dbController;
};

#endif // DIALOGSUBSYSTEMLISTEDITOR_H
