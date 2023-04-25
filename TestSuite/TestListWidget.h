#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>

class TestListWidget : public QWidget
{
	Q_OBJECT
public:
	TestListWidget(QWidget* parent);

public:
	void updateTestsList(const QStringList& tests);
	void clearTestsList();

	QStringList selectedTests() const;

signals:
	void testItemClicked(const QString& testName);


private slots:
	void testItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
	QLabel* m_testsPathLabel = nullptr;
	QTreeWidget* m_treeWidget = nullptr;


};

#endif // TESTLISTWIDGET_H
