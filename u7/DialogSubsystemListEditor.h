#ifndef DIALOGSUBSYSTEMLISTEDITOR_H
#define DIALOGSUBSYSTEMLISTEDITOR_H

#include <QDialog>
#include <QItemDelegate>
#include "../lib/DbController.h"
#include "../lib/Subsystem.h"

namespace Ui {
class DialogSubsystemListEditor;
}

class SubsystemListEditorDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	SubsystemListEditorDelegate(QObject *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};

class DialogSubsystemListEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSubsystemListEditor(DbController* pDbController, QWidget *parent = 0);
	~DialogSubsystemListEditor();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	bool askForSaveChanged();
	bool saveChanges();

protected:
	virtual void closeEvent(QCloseEvent* e) override;

private slots:
	void on_m_add_clicked();
	void on_m_remove_clicked();
	void on_buttonOk_clicked();
	void on_buttonCancel_clicked();
	void on_m_list_itemChanged(QTreeWidgetItem* item, int column);

public:
	enum Columns
	{
		Index,
		Key,
		SubsystemID,
		Caption
	};

private:
	Ui::DialogSubsystemListEditor* ui;

	bool m_modified = false;
	SubsystemListEditorDelegate* m_editorDelegate = nullptr;

	DbController* db();
	DbController* m_dbController;
};

#endif // DIALOGSUBSYSTEMLISTEDITOR_H
