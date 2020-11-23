#ifndef DIALOGTAGSEDITOR_H
#define DIALOGTAGSEDITOR_H

#include <QDialog>
#include "../lib/DbController.h"

namespace Ui {
	class DialogTagsEditor;
}

class DialogTagsEditor : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTagsEditor(DbController* pDbController, QWidget *parent = nullptr);
	~DialogTagsEditor();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	QStringList getTags() const;
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

private:

	bool m_modified = false;

	DbController* db();
	DbController* m_dbController;

private:
	Ui::DialogTagsEditor *ui;
};

#endif // DIALOGTAGSEDITOR_H
