#ifndef CHOOSEUFBDIALOG_H
#define CHOOSEUFBDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include "../VFrame30/UfbSchema.h"

namespace Ui {
class ChooseUfbDialog;
}

class ChooseUfbDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseUfbDialog(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs, QWidget* parent = 0);
	~ChooseUfbDialog();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	void fillTree();
	void itemSelected(QTreeWidgetItem* item, int column);
	void itemDoubleClicked(QModelIndex index);

public:
	std::shared_ptr<VFrame30::UfbSchema> result();

private:
	Ui::ChooseUfbDialog *ui;
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> m_ufbs;

	std::shared_ptr<VFrame30::UfbSchema> m_selectedUfb;
};

#endif // CHOOSEUFBDIALOG_H
