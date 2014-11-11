#include "Stable.h"
#include "BuildTabPage.h"
#include "Settings.h"
#include "../include/DbController.h"


//
//
// BuildTabPage
//
//

BuildTabPage::BuildTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	m_tasks.push_back(BuildTask(tr("Equipment configuration"), true));
	m_tasks.push_back(BuildTask(tr("Application logic"), true));
	m_tasks.push_back(BuildTask(tr("Signal database"), true));


	//
	// Controls
	//

	// Create Actions
	//
	CreateActions();

	// Left Side Widget
	//
	m_leftSideWidget = new QWidget();

	m_outputWidget = new QTextEdit();
	m_outputWidget->setReadOnly(true);
	m_outputWidget->document()->setUndoRedoEnabled(false);
	m_outputWidget->document()->setMaximumBlockCount(600);

	m_buildButton = new QPushButton(tr("Build..."));
	m_cancelButton = new QPushButton(tr("Cancel"));

	QGridLayout* leftWidgetLayout = new QGridLayout();

	leftWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 3);
	leftWidgetLayout->addWidget(m_buildButton, 1, 1);
	leftWidgetLayout->addWidget(m_cancelButton, 1, 2);

	leftWidgetLayout->setColumnStretch(0, 1);

	m_leftSideWidget->setLayout(leftWidgetLayout);

	// Right Side
	//
	m_taskTable = new QTableWidget(m_vsplitter);

	// V Splitter
	//
	m_vsplitter = new QSplitter(this);

	m_vsplitter->addWidget(m_taskTable);
	m_vsplitter->addWidget(m_leftSideWidget);

	m_vsplitter->setStretchFactor(0, 2);
	m_vsplitter->setStretchFactor(1, 1);

	m_vsplitter->restoreState(theSettings.m_buildTabPageSplitterState);

	//
	// Layouts
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_vsplitter);

	setLayout(pMainLayout);

	// Fill task table
	//
	m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_taskTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_taskTable->setShowGrid(false);

	m_taskTable->verticalHeader()->hide();
	m_taskTable->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_taskTable->fontMetrics().height() * 2.0));	// 1.4

	m_taskTable->horizontalHeader()->setHighlightSections(false);

	m_taskTable->setColumnCount(1);

	QStringList headers;
	headers.push_back(tr("Build task"));

	m_taskTable->setHorizontalHeaderLabels(headers);
	m_taskTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

	m_taskTable->setRowCount(static_cast<int>(m_tasks.size()));
	for (size_t i = 0; i < m_tasks.size(); i++)
	{
		QTableWidgetItem* ti = new QTableWidgetItem(m_tasks[i].name);

		ti->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		ti->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		ti->setCheckState(m_tasks[i].build ? Qt::Checked : Qt::Unchecked);

		m_taskTable->setItem(static_cast<int>(i), 0, ti);
	}

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &BuildTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &BuildTabPage::projectClosed);

	connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::build);
	connect(m_cancelButton, &QAbstractButton::clicked, this, &BuildTabPage::cancel);

	// Output Log
	//
	m_logTimerId = startTimer(25);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

BuildTabPage::~BuildTabPage()
{
	theSettings.m_buildTabPageSplitterState = m_vsplitter->saveState();
	theSettings.writeUserScope();
}

void BuildTabPage::CreateActions()
{
//	m_addSystemAction = new QAction(tr("System"), this);
//	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
//	m_addSystemAction->setEnabled(false);
//	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	return;
}

void BuildTabPage::writeOutputLog(const OutputLogItem& logItem)
{
	if (m_outputWidget == nullptr)
	{
		assert(m_outputWidget != nullptr);
		return;
	}

	// --
	//
	QString color;
	switch (logItem.level)
	{
	case Message:
		color = "black";
		break;
	case Success:
		color = "green";
		break;
	case Warning:
		color = "orange";
		break;
	case Error:
		color = "red";
		break;

	default:
		assert(false);
		color = "black";
	}

	QString s;
	if (logItem.message.isEmpty() == false)
	{
		s = "<font face=\"Verdana\" size=3 color=#808080>" + logItem.time.toString("hh:mm:ss:zzz   ") + "<font color=" + color + ">" + (logItem.bold ? QString("<b>") : QString()) + logItem.message;
	}

	m_outputWidget->append(s);
	return;
}

void BuildTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void BuildTabPage::timerEvent(QTimerEvent* event)
{

	if (event->timerId() == m_logTimerId &&
		m_outputLog.windowMessageListEmpty() == false &&
		m_outputWidget != nullptr)
	{
		std::list<OutputLogItem> messages;

		for (int i = 0; i < 10 && m_outputLog.windowMessageListEmpty() == false; i++)
		{
			messages.push_back(m_outputLog.popWindowMessages());
		}

		for (auto m = messages.begin(); m != messages.end(); ++m)
		{
			writeOutputLog(*m);
		}

		return;
	}

	return;
}

void BuildTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void BuildTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

void BuildTabPage::build()
{
	assert(m_taskTable);
	assert(m_taskTable->rowCount() == m_tasks.size());

	m_outputLog.clear();
	m_outputWidget->clear();

	std::vector<BuildTask> buildTask;

	for (size_t i = 0; i < m_tasks.size(); i++)
	{
		bool checked = m_taskTable->item(static_cast<int>(i), 0)->checkState() == Qt::Checked;
		m_tasks[i].build = checked;

		if (checked == true)
		{
			buildTask.push_back(m_tasks[i]);
		}
	}

	if (buildTask.empty() == true)
	{
		m_outputLog.writeError(tr("Nothing to build"));
		m_outputLog.writeMessage(tr("Select build task."));

		return;
	}

	return;
}

void BuildTabPage::cancel()
{
}

