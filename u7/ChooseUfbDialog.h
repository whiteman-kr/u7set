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
	explicit ChooseUfbDialog(QWidget *parent = 0, std::vector<std::shared_ptr<VFrame30::UfbSchema>> *ufbs = nullptr);
	~ChooseUfbDialog();

private:
	void fillTree();
	void itemSelected(QTreeWidgetItem *item, int column);

public:
	int selectedUfb = 0;

private:
	Ui::ChooseUfbDialog *ui;
	std::vector<std::shared_ptr<VFrame30::UfbSchema> > m_ufbs;
};

#endif // CHOOSEUFBDIALOG_H
