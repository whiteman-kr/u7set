#include "TestListWidget.h"
#include "../TestSuiteLib/ScriptRunner.h"
#include "AppConfigSettings.h"
#include <QKeyEvent>
#include <QHeaderView>

//
// TestTreeWidgetItem
//
TestTreeWidgetItem::TestTreeWidgetItem(const QString& caption):
	QTreeWidgetItem(QStringList() << caption)
{
}

QString TestTreeWidgetItem::fileName() const
{
	return m_fileName;
}

void TestTreeWidgetItem::setFileName(const QString& value)
{
	m_fileName = value;
}

QString TestTreeWidgetItem::function() const
{
	return m_function;
}

void TestTreeWidgetItem::setFunction(const QString& value)
{
	m_function = value;
}

bool TestTreeWidgetItem::permission() const
{
	return m_permission;
}

void TestTreeWidgetItem::setPermission(bool value)
{
	m_permission = value;
}

void TestTreeWidgetItem::saveCheckState()
{
	m_savedCheckState = checkState(0);
}

void TestTreeWidgetItem::restoreCheckState()
{
	if (m_savedCheckState.has_value() == true)
	{
		setCheckState(0, m_savedCheckState.value());
	}
}

void TestTreeWidgetItem::updatePermissionState(int columnStatus, bool selectionEnabled)
{
	setText(columnStatus, permission() ? TestSuite::ConstStrings::TEST_ALLOWED() : TestSuite::ConstStrings::TEST_DENIED());

	if (permission() == false)
	{
		setForeground(columnStatus, QBrush(Qt::red));
	}
	else
	{
		setForeground(columnStatus, QBrush(Qt::darkGreen));
	}

	if (selectionEnabled == true && permission() == true)
	{
		setFlags(flags() | Qt::ItemIsEnabled);
	}
	else
	{
		setFlags(flags() & ~Qt::ItemIsEnabled);
	}

	if (permission() == true)
	{
		// Enable checkbox
		
		restoreCheckState();
	}
	else
	{
		// Disable and clear checkbox
		saveCheckState();
		setCheckState(0, Qt::Unchecked);
	}
}

void TestTreeWidgetItem::setParentItemCheckState()
{
	if (parent() != nullptr)
	{
		// This is not parent item
		Q_ASSERT(false);
		return;
	}

	int childCheckedCount = 0;
	int childCounter = childCount();
	for (int c = 0; c < childCounter; c++)
	{
		if (child(c)->checkState(0) == Qt::Checked)
		{
			childCheckedCount++;
		}
	}

	Qt::CheckState parentState = Qt::Unchecked;
	if (childCheckedCount > 0)
	{
		if (childCheckedCount == childCounter)
		{
			parentState = Qt::Checked;
		}
		else
		{
			parentState = Qt::PartiallyChecked;
		}
	}
	setCheckState(0, parentState);
}


//
// TestTreeWidget
//
void TestTreeWidget::keyPressEvent(QKeyEvent* event)
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
	else
	{
		if (childCount > 0)
		{
			blockSignals(true);

			std::set<TestTreeWidgetItem*> parents;

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

				parents.insert(dynamic_cast<TestTreeWidgetItem*>(item->parent()));
			}

			for (TestTreeWidgetItem* parent : parents)
			{
				if (parent == nullptr)
				{
					Q_ASSERT(parent);
					break;
				}
				parent->setParentItemCheckState();
			}

			blockSignals(false);
		}
	}

	emit testSelectionChanged();
}

void TestTreeWidget::keyReleaseEvent(QKeyEvent* event)
{
	QTreeWidget::keyReleaseEvent(event);
}

TestListWidget::TestListWidget(const TestSuite::TestSuite& testSuite, TestSuiteLogFile& appLog, TestSuite::ConfigSettings& configuration, const TestSuite::TestScriptsStorage& tests, QWidget* parent) :
	QWidget(parent),
	m_testSuite(testSuite),
	m_appLog(appLog),
	m_configuration(configuration),
	m_tests(tests)
{
	m_testsPathLabel = new QLabel(tr("Test Scripts:"));
	m_testsPathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	m_testsPathLabel->setTextFormat(Qt::RichText);
	m_testsPathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_testsPathLabel->setOpenExternalLinks(true);

	m_treeWidget = new TestTreeWidget;
	m_treeWidget->setUniformRowHeights(true);
	m_treeWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QStringList headerLabels;
	headerLabels << tr("Test") << tr("Status") << tr("Result");
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
	connect(m_treeWidget, &TestTreeWidget::testSelectionChanged, [this]()
			{
				emit testSelectionChanged();
			});

	// Filter layout
	//
	QHBoxLayout* filterLayout = new QHBoxLayout();
	filterLayout->setContentsMargins(0, 0, 0, 0);
	filterLayout->setSpacing(0);

	m_filterEdit = new QLineEdit();
	m_filterEdit->setClearButtonEnabled(true);
	filterLayout->addWidget(m_filterEdit);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &TestListWidget::onFilterApply);
	connect(m_filterEdit, &QLineEdit::textChanged, [this](const QString& text)
			{
				if (text.isEmpty() == true)
				{
					onFilterApply();
				}
			});

	m_filterButton = new QPushButton(tr("Filter"));
	filterLayout->addWidget(m_filterButton);
	connect(m_filterButton, &QPushButton::clicked, this, &TestListWidget::onFilterApply);

	// Main layout
	//
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(m_testsPathLabel);
	layout->addWidget(m_treeWidget);
	layout->addLayout(filterLayout);

	setLayout(layout);
}

void TestListWidget::fillTestsTree()
{
	// Remember all previous top-level item states and their expand/check status
	//

	struct PrevItemState
	{
		PrevItemState() = default;
		explicit PrevItemState(TestTreeWidgetItem* item):
			expanded(item->isExpanded()),
			checked(item->checkState(0) == Qt::Checked)
		{
		}

		bool expanded{false};
		bool checked{false};
	};

	std::map<QString, PrevItemState> prevItemsStates; // Key is item name, value previous state

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			return;
		}

		// Remebmber if parent item is expanded
		//
		prevItemsStates[parentItem->fileName()] = PrevItemState(parentItem);

		// Remember if child items are checked
		//
		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			TestTreeWidgetItem* childItem = dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
			if (childItem == nullptr)
			{
				Q_ASSERT(childItem);
				return;
			}

			prevItemsStates[childItem->fileName() + childItem->function()] = PrevItemState(childItem);
		}
	}

	// Clear previous contents
	//
	m_treeWidget->clear();

	QString filterText = m_filterEdit->text();
	
	TestSuite::OutputControllerStub outputControllerStub;
	TestSuite::InputControllerStub inputControllerStub;
	TestSuite::ConfigSettings configSettings;
	TestSuite::TestController testController{configSettings, SoftwareInfo{}, nullptr, nullptr, nullptr, inputControllerStub, outputControllerStub, nullptr};
	ILogFileStub testLog;
	TestSuite::ControlStatus fakeStatus;
	QMutex fakeStatusMutex;

	const TestSuite::TestScript* globalScript = m_tests.globalScript();

	m_treeWidget->blockSignals(true);

	// Fill new tests
	//
	count = static_cast<int>(m_tests.count());
	for (int i = 0; i < count; i++)
	{
		const TestSuite::TestScript& script = m_tests.script(i);
		if (script.isGlobalScript() == true)
		{
			continue;
		}

		// Evaluate script to get functions list
		//
		TestSuite::ScriptRunner sr(script, globalScript, configSettings, testController, testLog, fakeStatus, fakeStatusMutex);

		const TestSuite::ScriptInfo& scriptInfo = sr.scriptInfo();
		if (scriptInfo.empty() == true)
		{
			continue;
		}

		// Check script tags
		//
		if (scriptInfo.checkScriptTags(m_configuration.scriptTags) == false)
		{
			continue;
		}

		// Create test item
		//
		TestTreeWidgetItem* scriptItem = new TestTreeWidgetItem(scriptInfo.scriptCaption);
		scriptItem->setFileName(scriptInfo.fileName);

		auto prevScriptItemIt = prevItemsStates.find(scriptInfo.fileName);

		for (const QString& function : scriptInfo.testsList)
		{
			QString functionCaption = scriptInfo.testCaption(function);

			if (functionCaption.contains(filterText) == false)
			{
				continue;
			}

			TestTreeWidgetItem* funcItem = new TestTreeWidgetItem(functionCaption);
			funcItem->setFileName(scriptInfo.fileName);
			funcItem->setFunction(function);

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
				auto prevFuncItemIt = prevItemsStates.find(scriptInfo.fileName + function);
				funcItem->setCheckState(0, prevFuncItemIt != prevItemsStates.end() && prevFuncItemIt->second.checked == true ? Qt::Checked : Qt::Unchecked);
			}

			funcItem->setPermission(m_testSuite.scriptPermission(scriptInfo.fileName) && m_testSuite.globalPermission());
			funcItem->updatePermissionState(Columns::Status, m_selectionEnabled);
		}

		if (scriptItem->childCount() == 0)
		{
			delete scriptItem;
			continue;
		}

		m_treeWidget->addTopLevelItem(scriptItem);
		scriptItem->setParentItemCheckState();
		scriptItem->setExpanded(prevScriptItemIt != prevItemsStates.end() && prevScriptItemIt->second.expanded == true);

		scriptItem->setPermission(m_testSuite.scriptPermission(scriptInfo.fileName) && m_testSuite.globalPermission());
		scriptItem->updatePermissionState(Columns::Status, m_selectionEnabled);
	}

	m_treeWidget->blockSignals(false);

	m_treeWidget->resizeColumnToContents(Columns::Caption);

	//
	if (AppConfigSettings().instance().useLocalScriptsPath() == true)
	{
		QString path = AppConfigSettings().instance().localScriptsPath();
		m_testsPathLabel->setText(tr("Test Scripts: <a href=\"%1\">%1</a>").arg(path));
	}
	else
	{
		m_testsPathLabel->setText(tr("Test Scripts:"));
	}
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
	m_selectionEnabled = enable;

	m_filterButton->setEnabled(enable);

	m_filterEdit->setEnabled(enable);

	m_treeWidget->blockSignals(true);

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			return;
		}

		if (enable == true && parentItem->permission() == true)
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

TestSuite::TestScriptSelection TestListWidget::getTestScriptSelection() const
{
	// Build TestScriptSelection structure which contains selected test functions
	//
	TestSuite::TestScriptSelection selection;

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		if (parentItem->permission() == true && parentItem->checkState(0) != Qt::Unchecked)
		{
			QStringList functions;

			int childCount = parentItem->childCount();
			for (int c = 0; c < childCount; c++)
			{
				TestTreeWidgetItem* childItem = dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
				if (parentItem->permission() == true && childItem->checkState(0) != Qt::Unchecked)
				{
					functions.push_back(childItem->function());
				}
			}

			selection.setSelectedFunctions(parentItem->fileName(), functions);
		}
	}

	return selection;
}

void TestListWidget::onScriptPermissionChanged(QString scriptFileName, bool permission)
{
	m_treeWidget->blockSignals(true);

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		if (parentItem->fileName() == scriptFileName)
		{
			parentItem->setPermission(permission);
			parentItem->updatePermissionState(Columns::Status, m_selectionEnabled);

			int childCount = parentItem->childCount();
			for (int c = 0; c < childCount; c++)
			{
				TestTreeWidgetItem* childItem = dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
				childItem->setPermission(permission);
				childItem->updatePermissionState(Columns::Status, m_selectionEnabled);
			}

			parentItem->setParentItemCheckState();
			break;
		}			
	}
		
	m_treeWidget->blockSignals(false);
	
	emit testSelectionChanged();
	return;
}

void TestListWidget::onNoPermissionsExist()
{
	m_treeWidget->blockSignals(true);

	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		parentItem->setPermission(true);
		parentItem->updatePermissionState(Columns::Status, m_selectionEnabled);

		int childCount = parentItem->childCount();
		for (int c = 0; c < childCount; c++)
		{
			TestTreeWidgetItem* childItem = dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
			childItem->setPermission(true);
			childItem->updatePermissionState(Columns::Status, m_selectionEnabled);
		}

		parentItem->setParentItemCheckState();
	}
		
	m_treeWidget->blockSignals(false);
	
	emit testSelectionChanged();
	return;
}

void TestListWidget::onTestStarted(QString scriptFileName, QString testFunction)
{
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		if (parentItem->fileName() == scriptFileName)
		{
			parentItem->setText(Columns::Result, TestSuite::ConstStrings::TEST_RUNNING());
			parentItem->setForeground(Columns::Result, QBrush(Qt::blue));
			if (parentItem->isExpanded() == false)
			{
				m_treeWidget->scrollToItem(parentItem);
			}

			int childCount = parentItem->childCount();
			for (int c = 0; c < childCount; c++)
			{
				TestTreeWidgetItem* childItem =  dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
				if (childItem->function() == testFunction)
				{
					childItem->setText(Columns::Result, TestSuite::ConstStrings::TEST_RUNNING());
					childItem->setForeground(Columns::Result, QBrush(Qt::blue));
					if (parentItem->isExpanded() == true)
					{
						m_treeWidget->scrollToItem(childItem);
					}
					break;
				}
			}
			break;
		}
	}
}

void TestListWidget::onTestFinished(QString scriptFileName, QString testFunction, bool result)
{
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
		if (parentItem == nullptr)
		{
			Q_ASSERT(parentItem);
			continue;
		}

		if (parentItem->fileName() == scriptFileName)
		{
			parentItem->setText(Columns::Result, QString());
			if (parentItem->isExpanded() == false)
			{
				m_treeWidget->scrollToItem(parentItem);
			}

			int childCount = parentItem->childCount();
			for (int c = 0; c < childCount; c++)
			{
				TestTreeWidgetItem* childItem =  dynamic_cast<TestTreeWidgetItem*>(parentItem->child(c));
				if (childItem->function() == testFunction)
				{
					childItem->setText(Columns::Result, result ? TestSuite::ConstStrings::TEST_PASSED() : TestSuite::ConstStrings::TEST_FAILED());

					if (result == false)
					{
						childItem->setForeground(Columns::Result, QBrush(Qt::red));
					}
					else
					{
						childItem->setForeground(Columns::Result, QBrush(Qt::darkGreen));
					}

					if (parentItem->isExpanded() == true)
					{
						m_treeWidget->scrollToItem(childItem);
					}

					break;
				}
			}
			break;
		}
	}
}

void TestListWidget::testItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	if (item == nullptr)
	{
		return;
	}

	TestTreeWidgetItem* testItem = dynamic_cast<TestTreeWidgetItem*>(item);
	if (testItem == nullptr)
	{
		Q_ASSERT(testItem);
		return;
	}

	emit testItemClicked(testItem->fileName(), testItem->function());
}

void TestListWidget::testItemChanged(QTreeWidgetItem* item, int column)
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
		TestTreeWidgetItem* parentTestItem = dynamic_cast<TestTreeWidgetItem*>(parentItem);
		if (parentTestItem == nullptr)
		{
			Q_ASSERT(parentTestItem);
			return;
		}

		m_treeWidget->blockSignals(true);
		parentTestItem->setParentItemCheckState();
		m_treeWidget->blockSignals(false);
	}

	emit testSelectionChanged();
}

void TestListWidget::contextMenuRequested()
{
	auto items = m_treeWidget->selectedItems();

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
					TestTreeWidgetItem* testItem = dynamic_cast<TestTreeWidgetItem*>(item);
					if (testItem == nullptr)
					{
						Q_ASSERT(testItem);
						return;
					}
					emit testItemClicked(testItem->fileName(), testItem->function());
				});
	}

	if (items.isEmpty() == false)
	{
		menu.addSeparator();
	}

	QAction* selectAllAction = menu.addAction(tr("Select All"));
	selectAllAction->setEnabled(m_selectionEnabled);
	connect(selectAllAction, &QAction::triggered, this, [this]()
			{
				for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
				{
					TestTreeWidgetItem* parentItem = dynamic_cast<TestTreeWidgetItem*>(m_treeWidget->topLevelItem(i));
					if (parentItem == nullptr)
					{
						Q_ASSERT(parentItem);
						return;
					}

					if (parentItem->permission() == true)
					{
						parentItem->setCheckState(0, Qt::Checked);
					}
				}
			});

	QAction* deselectAllAction = menu.addAction(tr("Unselect All"));
	deselectAllAction->setEnabled(m_selectionEnabled);
	connect(deselectAllAction, &QAction::triggered, this, [this]()
			{
				for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
				{
					m_treeWidget->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
				}
			});

	menu.addSeparator();

	QAction* expandAllAction = menu.addAction(tr("Expand All"));
	connect(expandAllAction, &QAction::triggered, this, [this]()
			{
				m_treeWidget->expandAll();
			});

	QAction* collapseAllAction = menu.addAction(tr("Collapse All"));
	connect(collapseAllAction, &QAction::triggered, this, [this]()
			{
				m_treeWidget->collapseAll();
			});

	menu.addSeparator();

	QAction* resizeToContentsAction = menu.addAction(tr("Auto-Resize"));
	connect(resizeToContentsAction, &QAction::triggered, this, [this]()
			{
				m_treeWidget->resizeColumnToContents(0);
				m_treeWidget->resizeColumnToContents(1);
				m_treeWidget->resizeColumnToContents(2);
			});

	menu.exec(QCursor::pos());
}

void TestListWidget::onFilterApply()
{
	QString filterText = m_filterEdit->text();

	if (filterText.isEmpty() == false)
	{
		m_filterEdit->setStyleSheet("QLineEdit { color: red }");
		m_filterButton->setStyleSheet("QPushButton { color: red }");
	}
	else
	{
		m_filterEdit->setStyleSheet(QString());
		m_filterButton->setStyleSheet(QString());
	}

	fillTestsTree();
}