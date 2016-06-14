#ifndef DIALOGCOLUMNS_H
#define DIALOGCOLUMNS_H

#include <QDialog>

namespace Ui {
class DialogColumns;
}


class DialogColumns : public QDialog
{
	Q_OBJECT

public:
	explicit DialogColumns(QWidget *parent, const QStringList& columnsNames, const std::vector<int>& columnsIndexes);
	~DialogColumns();

	std::vector<int> columnsIndexes();

private slots:
	void accept();

	void on_buttonAdd_clicked();

	void on_buttonAddAll_clicked();

	void on_buttonRemove_clicked();

	void on_buttonRemoveAll_clicked();

	void on_buttonUp_clicked();

	void on_buttonDown_clicked();

	void on_listExistingColumns_doubleClicked(const QModelIndex &index);

	void on_listSelectedColumns_doubleClicked(const QModelIndex &index);

private:
	Ui::DialogColumns *ui;

	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;
};

#endif // DIALOGCOLUMNS_H
