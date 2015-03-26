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
	MainTabPage(dbcontroller, parent),
	m_builder(&m_outputLog)
{
	assert(dbcontroller != nullptr);

	//
	// Controls
	//

	// Create Actions
	//
	CreateActions();

	// Right Side Widget
	//
	m_rightSideWidget = new QWidget();

	// Output windows
	//
	m_outputWidget = new QTextEdit();
	m_outputWidget->setReadOnly(true);
	m_outputWidget->document()->setUndoRedoEnabled(false);
	m_outputWidget->document()->setMaximumBlockCount(600);

	m_buildButton = new QPushButton(tr("Build..."));
	m_cancelButton = new QPushButton(tr("Cancel"));

	QGridLayout* rightWidgetLayout = new QGridLayout();

	rightWidgetLayout->addWidget(m_outputWidget, 0, 0, 1, 3);
	rightWidgetLayout->addWidget(m_buildButton, 1, 1);
	rightWidgetLayout->addWidget(m_cancelButton, 1, 2);

	rightWidgetLayout->setColumnStretch(0, 1);

	m_rightSideWidget->setLayout(rightWidgetLayout);

	// Left Side
	//
	m_settingsWidget = new QWidget(m_vsplitter);
	QVBoxLayout* settingsWidgetLayout = new QVBoxLayout();

	m_debugCheckBox = new QCheckBox(tr("Debug build"), m_settingsWidget);
	m_debugCheckBox->setChecked(true);

	settingsWidgetLayout->addWidget(m_debugCheckBox);

	m_settingsWidget->setLayout(settingsWidgetLayout);

	// V Splitter
	//
	m_vsplitter = new QSplitter(this);

	m_vsplitter->addWidget(m_settingsWidget);
	m_vsplitter->addWidget(m_rightSideWidget);

	m_vsplitter->setStretchFactor(0, 2);
	m_vsplitter->setStretchFactor(1, 1);

	m_vsplitter->restoreState(theSettings.m_buildTabPageSplitterState);

	//
	// Layouts
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_vsplitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &BuildTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &BuildTabPage::projectClosed);

	connect(m_buildButton, &QAbstractButton::clicked, this, &BuildTabPage::build);
	connect(m_cancelButton, &QAbstractButton::clicked, this, &BuildTabPage::cancel);

	connect(&m_builder, &ProjectBuilder::buildStarted, this, &BuildTabPage::buildStarted);
	connect(&m_builder, &ProjectBuilder::buildFinished, this, &BuildTabPage::buildFinished);

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
	m_outputLog.clear();
	m_outputWidget->clear();

	bool debug = m_debugCheckBox->isChecked();

	m_builder.start(
		db()->currentProject().projectName(),
		db()->host(),
		db()->port(),
		db()->serverUsername(),
		db()->serverPassword(),
		db()->currentUser().username(),
		db()->currentUser().password(),
		debug);

	return;
}

void BuildTabPage::cancel()
{
	m_builder.stop();
}

void BuildTabPage::buildStarted()
{
	m_buildButton->setEnabled(false);
	m_cancelButton->setEnabled(true);
}

void BuildTabPage::buildFinished()
{
	m_buildButton->setEnabled(true);
	m_cancelButton->setEnabled(false);
}
