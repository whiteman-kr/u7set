#ifndef DIALOGSIGNALSEARCH_H
#define DIALOGSIGNALSEARCH_H

#include "../lib/AppSignal.h"
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
	void on_DialogSignalSearch_finished(int result);
	void prepareContextMenu(const QPoint& pos);

	void on_signalsTree_doubleClicked(const QModelIndex &index);

protected:

private:
	void search();

private:
	Ui::DialogSignalSearch *ui;

	static QString m_signalId;

	std::vector<AppSignalParam> m_signals;
};

#endif // DIALOGSIGNALSEARCH_H
