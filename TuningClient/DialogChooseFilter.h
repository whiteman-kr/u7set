#ifndef DIALOGCHOOSEFILTER_H
#define DIALOGCHOOSEFILTER_H

namespace AppSignalLists
{
	class AppSignalList;
	class AppSignalListSet;
}

class QListWidget;


class DialogChooseFilter : public QDialog
{
	Q_OBJECT

public:
	DialogChooseFilter(const AppSignalLists::AppSignalListSet& appSignalLists, const QStringList& systemTags, QWidget* parent);
	
	QUuid chosenFilterUuid() const;

private slots:
	virtual void accept();

private:
	QListWidget* m_listBox = nullptr;
	QUuid m_chosenFilter;
};

#endif // DIALOGCHOOSEFILTER_H
