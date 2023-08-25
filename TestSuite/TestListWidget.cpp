#include "TestListWidget.h"
#include "../TestSuiteLib/ScriptRunner.h"

void TestTreeWidget::setParentItemsCheckState()
{
	int count = topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = topLevelItem(i);

		int childCheckedCount = 0;
		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			if (parentItem->child(c)->checkState(0) == Qt::Checked)
			{
				childCheckedCount++;
			}
		}

		Qt::CheckState parentState = Qt::Unchecked;
		if (childCheckedCount > 0)
		{
			if (childCheckedCount == childCount)
			{
				parentState = Qt::Checked;
			}
			else
			{
				parentState = Qt::PartiallyChecked;
			}
		}
		parentItem->setCheckState(0, parentState);
	}
}

void TestTreeWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() != Qt::Key_Space)
	{
		QTreeWidget::keyPressEvent(event);
		return;
	}

	int parentCount = 0;
	int childCount = 0;

	QList<QTreeWidgetItem*> items = selectedItems();
	for (const QTreeWidgetItem* item : items)
	{
		if (item->parent() == nullptr)
		{
			parentCount++;
		}
		else
		{
			childCount++;
		}
	}

	// If mixed elements are checked (parent and children) - do nothing
	if (parentCount != 0 && childCount != 0)
	{
		return;
	}

	if (parentCount > 0)
	{
		// Toggle parent items. Signals are NOT blocked to process child items!!!

		for (QTreeWidgetItem* item : items)
		{
			if (item->checkState(0) != Qt::Checked)
			{
				item->setCheckState(0, Qt::Checked);
			}
			else
			{
				item->setCheckState(0, Qt::Unchecked);
			}
		}
	}

	if (childCount > 0)
	{
		blockSignals(true);

		for (QTreeWidgetItem* item : items)
		{
			if (item->checkState(0) != Qt::Checked)
			{
				item->setCheckState(0, Qt::Checked);
			}
			else
			{
				item->setCheckState(0, Qt::Unchecked);
			}
		}

		setParentItemsCheckState();

		blockSignals(false);
	}
}

void TestTreeWidget::keyReleaseEvent(QKeyEvent *event)
{
	QTreeWidget::keyReleaseEvent(event);
}

TestListWidget::TestListWidget(QWidget* parent):
	QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout;

	m_testsPathLabel = new QLabel("Tests Path: Not loaded");
	m_testsPathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	m_testsPathLabel->setTextFormat(Qt::RichText);
	m_testsPathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_testsPathLabel->setOpenExternalLinks(true);

	m_treeWidget = new TestTreeWidget;
	m_treeWidget->setUniformRowHeights(true);
	m_treeWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QStringList headerLabels;
	headerLabels << tr("Test") << tr("Result");
	m_treeWidget->setHeaderLabels(headerLabels);
	m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_treeWidget->setExpandsOnDoubleClick(false);
	m_treeWidget->header()->setStretchLastSection(true);

	QByteArray headerState = QSettings().value("TestsListWidget/headerState").toByteArray();
	m_treeWidget->header()->restoreState(headerState);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TestListWidget::testItemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemChanged, this, &TestListWidget::testItemChanged);

	layout->addWidget(m_testsPathLabel);
	layout->addWidget(m_treeWidget);

	setLayout(layout);
}

void TestListWidget::updateTestsList(const TestSuite::TestScriptsStorage& tests)
{
	TestSuite::OutputControllerStub outputControllerStub;
	TestSuite::InputControllerStub inputControllerStub;
	TestSuite::TestController testController{inputControllerStub, outputControllerStub, nullptr};
	TestSuite::TestLogStub testLog;
	TestSuite::ControlStatus fakeStatus;
	QMutex fakeStatusMutex;

	clearTestsList();

	int count = static_cast<int>(tests.count());
	for (int i = 0; i < count; i++)
	{
		TestSuite::ScriptRunner sr(testController, testLog, fakeStatus, fakeStatusMutex);
		const TestSuite::TestScript& script = tests.script(i);

		QTreeWidgetItem* scriptItem = new QTreeWidgetItem(QStringList() << script.fileName());
		scriptItem->setCheckState(0, Qt::Checked);
		scriptItem->setData(ColumnsData::ScriptName, Qt::UserRole, script.fileName());
		m_treeWidget->addTopLevelItem(scriptItem);

		QStringList functions;
		QString errorMsg;
		if (sr.getScriptTestFunctions(script, functions, errorMsg) == false)
		{
			continue;
		}

		for (const QString& func : functions)
		{
			QTreeWidgetItem* funcItem = new QTreeWidgetItem(QStringList() << func);
			funcItem->setCheckState(0, Qt::Checked);
			funcItem->setData(ColumnsData::ScriptName, Qt::UserRole, script.fileName());
			funcItem->setData(ColumnsData::TestFunction, Qt::UserRole, func);
			scriptItem->addChild(funcItem);
		}

		scriptItem->setExpanded(true);
	}

	m_treeWidget->resizeColumnToContents(Columns::Caption);
}

void TestListWidget::clearTestsList()
{
	m_treeWidget->clear();
}


void TestListWidget::clearTestsResults()
{
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);

		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			QTreeWidgetItem* childItem = parentItem->child(c);
			childItem->setText(Columns::Result, QString());
		}
	}
}

void TestListWidget::fillTestScriptFilter(TestSuite::TestScriptFilter& filter) const
{
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);
		if (parentItem->checkState(0) == Qt::Unchecked)
		{
			continue;
		}

		QString scriptFileName = parentItem->data(ColumnsData::ScriptName, Qt::UserRole).toString();
		if (scriptFileName.isEmpty() == true)
		{
			Q_ASSERT(false);
			continue;
		}

		QStringList functions;

		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			QTreeWidgetItem* childItem = parentItem->child(c);
			if (childItem->checkState(0) == Qt::Unchecked)
			{
				continue;
			}

			QString func = childItem->data(ColumnsData::TestFunction, Qt::UserRole).toString();
			if (func.isEmpty() == true)
			{
				Q_ASSERT(false);
				continue;
			}

			functions.push_back(func);
		}

		filter.setTestFunctions(scriptFileName, functions);
	}

	return;
}

void TestListWidget::onTestFinished(QString scriptFileName, QString testFunction, bool result)
{
	//qDebug() << "Test function finished:" << scriptFileName << testFunction;

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);
		if (parentItem->data(ColumnsData::ScriptName, Qt::UserRole).toString() == scriptFileName)
		{
			int childCount = parentItem->childCount();
			for (int c = 0; c < childCount; c++)
			{
				QTreeWidgetItem* childItem = parentItem->child(c);
				if (childItem->data(ColumnsData::TestFunction, Qt::UserRole).toString() == testFunction)
				{
					childItem->setText(Columns::Result, result ? "PASS" : "FAIL");

					if (result == false)
					{
						childItem->setForeground(Columns::Result, QBrush(Qt::red));
					}
					else
					{
						childItem->setForeground(Columns::Result, QBrush(Qt::darkGreen));
					}

					break;
				}
			}

			return;
		}
	}
}

void TestListWidget::testItemDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
	if (item == nullptr)
	{
		return;
	}
	if (item->parent() == nullptr)	// Check if this is top level item
	{
		emit testItemClicked(item->text(Columns::Caption));
	}
}

void TestListWidget::testItemChanged(QTreeWidgetItem *item, int column)
{
	if (column != 0)
	{
		return;
	}

	QTreeWidgetItem* parentItem = item->parent();

	if (parentItem == nullptr)
	{
		// Script item changed - check or uncheck all child items
		//
		m_treeWidget->blockSignals(true);

		Qt::CheckState state = item->checkState(0);
		if (state == Qt::PartiallyChecked)
		{
			state = Qt::Checked;

			item->setCheckState(0, state);
		}

		int childCount = item->childCount();
		for (int i = 0; i < childCount; i++)
		{
			item->child(i)->setCheckState(0, state);
		}

		m_treeWidget->blockSignals(false);
	}
	else
	{
		// Test item changed - check or uncheck parent items
		//
		m_treeWidget->blockSignals(true);
		m_treeWidget->setParentItemsCheckState();
		m_treeWidget->blockSignals(false);
	}
}
