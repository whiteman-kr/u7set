#ifndef CHOOSEAFBDIALOG_H
#define CHOOSEAFBDIALOG_H

#include <QDialog>
#include "../VFrame30/Fbl.h"

namespace Ui {
class ChooseAfbDialog;
}

class ChooseAfbDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseAfbDialog(const std::vector<std::shared_ptr<Afbl::AfbElement> > &elements, QWidget *parent = 0);
	~ChooseAfbDialog();

	int index();

private slots:
	void on_btnOk_clicked();

	void on_btnCancel_clicked();

private:
	Ui::ChooseAfbDialog *ui;
	std::vector<std::shared_ptr<Afbl::AfbElement>> elements;
	int m_index = -1;

	static int m_lastSelectedIndex;
};

#endif // CHOOSEAFBDIALOG_H
