#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>

class TestListWidget : public QWidget
{
public:
	TestListWidget(QWidget* parent);

public:
	void updateTestsList(const QStringList& tests);
	void clearTestsList();

	QStringList selectedTests() const;

private:
	QLabel* m_testsPathLabel = nullptr;
	QTreeWidget* m_treeWidget = nullptr;


};

#endif // TESTLISTWIDGET_H
