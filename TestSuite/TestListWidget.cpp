#include "TestListWidget.h"
#include "../TestSuiteLib/ScriptRunner.h"
#include "AppConfigSettings.h"

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

	m_testsPathLabel = new QLabel(tr("Test Scripts:"));
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
	m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TestListWidget::testItemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemChanged, this, &TestListWidget::testItemChanged);
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &TestListWidget::contextMenuRequested);

	layout->addWidget(m_testsPathLabel);
	layout->addWidget(m_treeWidget);

	setLayout(layout);
}

void TestListWidget::fillTestsTree(const TestSuite::TestScriptsStorage& tests)
{
	TestSuite::OutputControllerStub outputControllerStub;
	TestSuite::InputControllerStub inputControllerStub;
	TestSuite::ConfigSettings configSettings;
	TestSuite::TestController testController{configSettings, SoftwareInfo{}, nullptr, nullptr, nullptr, inputControllerStub, outputControllerStub, nullptr};
	ILogFileStub testLog;
	TestSuite::ControlStatus fakeStatus;
	QMutex fakeStatusMutex;

	// Remember all previous top-level item states and their expand/check status
	//
	std::map<QString, bool> prevItemsStates;	// Key is item name, value is "expanded" for file items, "checked" for test items
	
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			return;
		}
		prevItemsStates[parentItem->text(Columns::Caption)] = parentItem->isExpanded();

		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			QTreeWidgetItem* childItem = parentItem->child(c);
			if (childItem == nullptr)
			{
				Q_ASSERT(childItem);
				return;
			}
			prevItemsStates[parentItem->text(Columns::Caption) + childItem->text(Columns::Caption)] = childItem->checkState(0) == Qt::Checked;
		}
	}

	// Clear previous contents
	//
	clearTestsList();

	// Fill new tests
	//
	count = static_cast<int>(tests.count());
	for (int i = 0; i < count; i++)
	{
		TestSuite::ScriptRunner sr(testController, testLog, fakeStatus, fakeStatusMutex);

		const TestSuite::TestScript& script = tests.script(i);
		if (script.isGlobalScript() == true)
		{
			continue;
		}

		QTreeWidgetItem* scriptItem = new QTreeWidgetItem(QStringList() << script.fileName());
		scriptItem->setData(ColumnsData::ScriptName, Qt::UserRole, script.fileName());
		m_treeWidget->addTopLevelItem(scriptItem);

		QStringList functions;
		QString errorMsg;
		if (sr.getScriptTestFunctions(script, functions, errorMsg) == false)
		{
			continue;
		}

		auto prevScriptItemIt = prevItemsStates.find(script.fileName());

		for (const QString& func : functions)
		{
			QTreeWidgetItem* funcItem = new QTreeWidgetItem(QStringList() << func);
			funcItem->setData(ColumnsData::ScriptName, Qt::UserRole, script.fileName());
			funcItem->setData(ColumnsData::TestFunction, Qt::UserRole, func);
			scriptItem->addChild(funcItem);

			if (prevScriptItemIt == prevItemsStates.end())
			{
				// If this function is from the new file - check it
				//
				funcItem->setCheckState(0, Qt::Checked);
			}
			else
			{
				// If this function is from existing file - check it if it was selected earlier
				//
				auto prevFuncItemIt = prevItemsStates.find(script.fileName() + func);
				funcItem->setCheckState(0, prevFuncItemIt != prevItemsStates.end() && prevFuncItemIt->second == true ? Qt::Checked : Qt::Unchecked);
			}
		}

		scriptItem->setExpanded(prevScriptItemIt == prevItemsStates.end() || prevScriptItemIt->second == true);
	}

	m_treeWidget->resizeColumnToContents(Columns::Caption);

	// 
	if (theSettings.useLocalScriptsPath() == true)
	{
		QString path = theSettings.localScriptsPath();
		m_testsPathLabel->setText(tr("Test Scripts: <a href=\"%1\">%1</a>").arg(path));
	}
	else
	{
		m_testsPathLabel->setText(tr("Test Scripts:"));
	}
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

void TestListWidget::setSelectionEnabled(bool enable)
{
	m_treeWidget->blockSignals(true);

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);

		if (enable == true)
		{
			parentItem->setFlags(parentItem->flags() | Qt::ItemIsEnabled);
		}
		else
		{
			parentItem->setFlags(parentItem->flags() & ~Qt::ItemIsEnabled);
		}
	}

	m_treeWidget->blockSignals(false);
}

TestSuite::TestScriptSelection TestListWidget::testScriptSelection() const
{
	TestSuite::TestScriptSelection selection;

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* parentItem = m_treeWidget->topLevelItem(i);
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		QString scriptFileName = parentItem->data(ColumnsData::ScriptName, Qt::UserRole).toString();
		if (scriptFileName.isEmpty() == true)
		{
			Q_ASSERT(false);
			continue;
		}

		if (parentItem->checkState(0) != Qt::Unchecked)
		{
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

			selection.setSelectedFunctions(scriptFileName, functions);
		}
	}

	return selection;
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
					childItem->setText(Columns::Result, result ? tr("PASS") : tr("FAIL"));

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
		QString fileName = item->text(Columns::Caption);
		
		emit testItemClicked(fileName, QString());
	}
	else
	{
		QString fileName = item->parent()->text(Columns::Caption);
		QString function = item->text(Columns::Caption);
		
		emit testItemClicked(fileName, function);
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

	emit testSelectionChanged();
}

void TestListWidget::contextMenuRequested()
{
	auto items = m_treeWidget->selectedItems();
	if (items.isEmpty() == true)
	{
		return;
	}

	QMenu menu(this);

	for (auto item : items)
	{
		QAction* a;
		if (item->parent() == nullptr)
		{
			a = menu.addAction(tr("View script '%1'").arg(item->text(Columns::Caption)));
		}
		else
		{
			a = menu.addAction(tr("View function '%1'").arg(item->text(Columns::Caption)));
		}

		connect(a, &QAction::triggered, this, [this, item]()
			{
				if (item->parent() == nullptr)	// Check if this is top level item (script)
				{
					emit testItemClicked(item->text(Columns::Caption), QString());
				}
				else
				{
					emit testItemClicked(item->parent()->text(Columns::Caption), item->text(Columns::Caption));
				}
			}
		);
	}

	menu.exec(QCursor::pos());
}