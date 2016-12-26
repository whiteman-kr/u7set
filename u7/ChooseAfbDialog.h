#ifndef CHOOSEAFBDIALOG_H
#define CHOOSEAFBDIALOG_H

#include <QDialog>
#include "../VFrame30/Afb.h"

namespace Ui {
class ChooseAfbDialog;
}

class ChooseAfbDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseAfbDialog(const std::vector<std::shared_ptr<Afb::AfbElement>> &m_elements, QWidget* parent = 0);
	~ChooseAfbDialog();

	int index();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
    void fillTree();
	int getSelectedIndex();
    QString getSelectedCategory();

	virtual void reject();
	virtual void closeEvent(QCloseEvent* e);

private slots:
	void on_btnOk_clicked();
	void on_btnCancel_clicked();
	void on_m_afbTree_itemSelectionChanged();
	void on_m_afbTree_itemDoubleClicked(QTreeWidgetItem* item, int column);

    void on_editQuickSearch_textEdited(const QString &arg1);

private:
	Ui::ChooseAfbDialog *ui;
	std::vector<std::shared_ptr<Afb::AfbElement>> m_elements;

    static QString m_lastSelectedCategory;
	static int m_lastSelectedIndex;
	static Qt::SortOrder m_lastSortOrder;

    static QString AllCategoryName;
};

#endif // CHOOSEAFBDIALOG_H
