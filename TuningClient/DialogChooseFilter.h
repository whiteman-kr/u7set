#ifndef DIALOGCHOOSEFILTER_H
#define DIALOGCHOOSEFILTER_H

#include <TuningLib/TuningFilter.h>

class QListWidget;


class DialogChooseFilter : public QDialog
{
	Q_OBJECT

public:
	DialogChooseFilter(QWidget* parent, TuningFilters::TuningFilter* parentFilter, TuningFilters::TuningFilter::InterfaceType interfaceType, TuningFilters::TuningFilter::Source source);

	TuningFilters::TuningFilter* chosenFilter() const;

private slots:
	virtual void accept();

private:

	QListWidget* m_listBox = nullptr;
	TuningFilters::TuningFilter* m_chosenFilter = nullptr;
};

#endif // DIALOGCHOOSEFILTER_H
