#ifndef DIALOGTAGSEDITOR_H
#define DIALOGTAGSEDITOR_H

#include <QDialog>
#include "../lib/DbController.h"

namespace Ui {
	class DialogTagsEditor;
}

class DialogTagsEditorDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	DialogTagsEditorDelegate(QObject *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};

class DialogTagsEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTagsEditor(DbController* pDbController, QWidget *parent = nullptr);
	~DialogTagsEditor();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	std::vector<DbTag> getTags() const;
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
	void on_remove_shortcut();

	void on_m_up_clicked();

	void on_m_down_clicked();

private:

	bool m_modified = false;

	DbController* db();
	DbController* m_dbController;

private:
	Ui::DialogTagsEditor *ui;
};

#endif // DIALOGTAGSEDITOR_H
