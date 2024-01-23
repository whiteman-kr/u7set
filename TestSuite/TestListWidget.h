#ifndef TESTLISTWIDGET_H
#define TESTLISTWIDGET_H

#include <QTreeWidget>
#include "../TestSuiteLib/TestSuite.h"
#include "../TestSuiteLib/TestController.h"
#include "../TestSuiteLib/TestLog.h"
#include "../TestSuiteLib/TestScriptsStorage.h"
#include "TestSuiteLog.h"

class TestTreeWidgetItem : public QTreeWidgetItem
{
public:
	TestTreeWidgetItem(const QString& caption);

	QString fileName() const;
	void setFileName(const QString& value);

	QString function() const;
	void setFunction(const QString& value);

	bool permission() const;
	void setPermission(bool value);

	void saveCheckState();
	void restoreCheckState();

	void updatePermissionState(int columnStatus, bool selectionEnabled);

	void setParentItemCheckState();

private:
	QString m_caption;
	QString m_fileName;
	QString m_function;
	bool m_permission = true;
	std::optional<Qt::CheckState> m_savedCheckState;
};


class TestTreeWidget : public QTreeWidget
{
	Q_OBJECT

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
	TestListWidget(const TestSuite::TestSuite& testSuite, TestSuiteLogFile& appLog, TestSuite::ConfigSettings& configuration, const TestSuite::TestScriptsStorage& tests, QWidget* parent);

	enum Columns
	{
		Caption,
		Status,
		Result
	};
	enum ColumnsData
	{
		ScriptTreeItem,	// First column is a pointer to TestTreeScript structure
		TestFunction
	};

public:
	void fillTestsTree();
	void clearTestsResults();
	
	void setSelectionEnabled(bool enable);

	TestSuite::TestScriptSelection getTestScriptSelection() const;

public slots:
	void onScriptPermissionChanged(QString scriptFileName, bool result);
	void onNoPermissionsExist();

	void onTestStarted(QString scriptFileName, QString testFunction);
	void onTestFinished(QString scriptFileName, QString testFunction, bool result);

signals:
	void testSelectionChanged();
	void testItemClicked(const QString& scriptName, const QString& functionName);
	

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

	bool m_selectionEnabled = false;
	
	const TestSuite::TestSuite& m_testSuite;
	const TestSuite::ConfigSettings& m_configuration;

	const TestSuite::TestScriptsStorage& m_tests;
	TestSuiteLogFile& m_appLog;
};

#endif // TESTLISTWIDGET_H
