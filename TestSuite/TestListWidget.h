#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>
#include "../TestSuiteLib/TestController.h"
#include "../TestSuiteLib/TestLog.h"
#include "../TestSuiteLib/TestScriptsStorage.h"
#include "TestSuiteLog.h"

class TestTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	void setParentItemsCheckState();

private:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;

signals:
	void testSelectionChanged();

};


class TestListWidget : public QWidget
{
	Q_OBJECT
public:
	TestListWidget(TestSuiteLogFile& appLog, QWidget* parent);

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
	void fillTestsTree(const TestSuite::TestScriptsStorage& tests);
	void clearTestsList();

	void clearTestsResults();

	void setSelectionEnabled(bool enable);

	TestSuite::TestScriptSelection testScriptSelection() const;

public slots:
	void onTestFinished(QString scriptFileName, QString testFunction, bool result);

signals:
	void testSelectionChanged();
	void testItemClicked(const QString& scriptName, const QString& functionName);

private slots:
	void testItemDoubleClicked(QTreeWidgetItem *item, int column);
	void testItemChanged(QTreeWidgetItem *item, int column);
	void contextMenuRequested();

private:
	QLabel* m_testsPathLabel = nullptr;
	TestTreeWidget* m_treeWidget = nullptr;
	TestSuiteLogFile& m_appLog;
};

#endif // TESTLISTWIDGET_H
