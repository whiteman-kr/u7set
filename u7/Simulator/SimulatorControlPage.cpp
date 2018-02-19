#include "SimulatorControlPage.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>

SimulatorControlPage::SimulatorControlPage(std::shared_ptr<Sim::LogicModule> logicModule, QWidget* parent)
	: SimulatorBasePage(parent),
	m_logicModule(logicModule)
{
	assert(m_logicModule);

	// --
	//
	m_subsystemIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_equipmentIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_channelLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

	m_schemaFilterEdit->setPlaceholderText("Filter");
	m_schemaFilterEdit->setClearButtonEnabled(true);


	m_splitter->setChildrenCollapsible(false);

	// Left side
	//
	{
		QWidget* leftWidget = new QWidget(m_splitter);

		QGridLayout* layout = new QGridLayout;
		leftWidget->setLayout(layout);

		layout->addWidget(m_subsystemIdLabel, 0, 0, 1, 3);
		layout->addWidget(m_equipmentIdLabel, 1, 0, 1, 3);
		layout->addWidget(m_channelLabel, 2, 0, 1, 3);

		// Add spacer
		//
		QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
		layout->addItem(spacer, 3, 0, 1, 3);

		// Buttons  Signals, Memory, Code
		//
		layout->addWidget(m_signalsButton, 4, 0, 1, 1);
		layout->addWidget(m_memoryButton, 4, 1, 1, 1);
		layout->addWidget(m_codeButton, 4, 2, 1, 1);

		m_splitter->addWidget(leftWidget);
		m_splitter->setStretchFactor(0, 1);
	}

	// Right side
	//
	{
		QWidget* rightWidget = new QWidget(m_splitter);

		QGridLayout* layout = new QGridLayout;
		rightWidget->setLayout(layout);

		layout->addWidget(m_schemasLabel, 0, 0, 1, 3);
		layout->addWidget(m_schemasList, 1, 0, 1, 3);
		layout->addWidget(m_schemaFilterEdit, 2, 0, 1, 3);

		m_splitter->addWidget(rightWidget);
		m_splitter->setStretchFactor(1, 3);
	}

	QGridLayout* layout = new QGridLayout;
	this->setLayout(layout);
	layout->addWidget(m_splitter, 0, 0, 1, 1);

	// --
	//
	updateLogicModuleInfoInfo();

	return;
}

void SimulatorControlPage::updateLogicModuleInfoInfo()
{
	if (m_logicModule == nullptr)
	{
		return;
	}

	const Hardware::LogicModuleInfo& lmInfo = m_logicModule->logicModuleInfo();

	m_subsystemIdLabel->setText(QString("Subsystem: %1").arg("TODO"));

	m_equipmentIdLabel->setText(QString("EquipmnetID: %1").arg(lmInfo.equipmentId));

	m_channelLabel->setText(QString("LmNumber: %1, Channel: %2")
								.arg(lmInfo.lmNumber)
								.arg(QChar('A' + static_cast<char>(lmInfo.channel))));

	return;
}

QString SimulatorControlPage::equipmnetId() const
{
	if (m_logicModule == nullptr)
	{
		assert(m_logicModule);
		return QString();
	}

	return m_logicModule->equipmentId();
}
