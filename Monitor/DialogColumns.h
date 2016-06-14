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

	std::vector<int> columnIndexes();

private slots:
	void accept();

private:
	Ui::DialogColumns *ui;

	QStringList m_columnsNames;
	std::vector<int> m_columnsIndexes;
};

#endif // DIALOGCOLUMNS_H
