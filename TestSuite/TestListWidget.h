#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>
#include "../TestSuiteLib/TestController.h"
#include "../TestSuiteLib/TestLog.h"
#include "../TestSuiteLib/TestScriptsStorage.h"
#include "TestSuiteLog.h"

struct TestTreeFunction
{
	QString caption;
	QString function;
};

struct TestTreeScript
{
	QString caption;
	QString fileName;
	std::vector<TestTreeFunction> functions;
};

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
	void setTests(const TestSuite::TestScriptsStorage& tests);

	void clearTestsResults();

	void setSelectionEnabled(bool enable);

	TestSuite::TestScriptSelection testScriptSelection() const;

public slots:
	void onTestFinished(QString scriptFileName, QString testFunction, bool result);

signals:
	void testSelectionChanged();
	void testItemClicked(const QString& scriptName, const QString& functionName);

private:
	void fillTestsTree();

private slots:
	void testItemDoubleClicked(QTreeWidgetItem *item, int column);
	void testItemChanged(QTreeWidgetItem *item, int column);
	void contextMenuRequested();
	void onFilterApply();

private:
	QLabel* m_testsPathLabel = nullptr;
	TestTreeWidget* m_treeWidget = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QPushButton* m_filterButton = nullptr;

	std::vector<TestTreeScript> m_scriptItems;

	bool m_selectionEnabled = false;
	
	TestSuiteLogFile& m_appLog;
};

#endif // TESTLISTWIDGET_H
