#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>
#include "../TestSuiteLib/TestController.h"
#include "../TestSuiteLib/TestLog.h"
#include "../TestSuiteLib/TestScriptsStorage.h"

class TestTreeWidget : public QTreeWidget
{
public:
	void setParentItemsCheckState();

private:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
};


class TestListWidget : public QWidget
{
	Q_OBJECT
public:
	TestListWidget(QWidget* parent);

	enum Columns
	{
		Caption,
		Result
	};
	enum ColumnsData
	{
		ScriptName,
		TestFunction
	};

public:
	void updateTestsList(const TestSuite::TestScriptsStorage& tests);
	void clearTestsList();

	void clearTestsResults();

	void fillTestScriptFilter(TestSuite::TestScriptFilter& filter) const;

public slots:
	void onTestFinished(QString scriptFileName, QString testFunction, bool result);

signals:
	void testItemClicked(const QString& testName);

private slots:
	void testItemDoubleClicked(QTreeWidgetItem *item, int column);
	void testItemChanged(QTreeWidgetItem *item, int column);

private:
	QLabel* m_testsPathLabel = nullptr;
	TestTreeWidget* m_treeWidget = nullptr;
};

#endif // TESTLISTWIDGET_H
