#ifndef DIALOGSIGNALSEARCH_H
#define DIALOGSIGNALSEARCH_H

#include "../lib/Signal.h"
#include <QDialog>

namespace Ui {
class DialogSignalSearch;
}

class DialogSignalSearch : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalSearch(QWidget *parent = 0);
	~DialogSignalSearch();

private slots:
	void on_editSignalID_textEdited(const QString &arg1);
	void on_comboSignalType_currentIndexChanged(int index);
	void on_DialogSignalSearch_finished(int result);
	void prepareContextMenu(const QPoint& pos);

protected:

private:
	void search();

private:
	Ui::DialogSignalSearch *ui;

	static QString m_signalID;
	static int m_signalTypeIndex;

	std::vector<Signal> m_signals;
};

#endif // DIALOGSIGNALSEARCH_H
