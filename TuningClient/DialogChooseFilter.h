#ifndef DIALOGCHOOSEFILTER_H
#define DIALOGCHOOSEFILTER_H

#include "../lib/Tuning/TuningFilter.h"

class QListWidget;

class DialogChooseFilter : public QDialog
{
	Q_OBJECT

public:
	DialogChooseFilter(QWidget* parent, TuningFilter* parentFilter, TuningFilter::InterfaceType interfaceType, TuningFilter::Source source);

	TuningFilter* chosenFilter() const;

private slots:
	virtual void accept();

private:

	QListWidget* m_listBox = nullptr;

	TuningFilter* m_chosenFilter = nullptr;
};

#endif // DIALOGCHOOSEFILTER_H
