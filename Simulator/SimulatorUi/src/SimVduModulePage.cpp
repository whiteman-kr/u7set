#include "SimVduModulePage.h"

#include "SimWidgetPrivate.h"
#include <CommonLib/ConstStrings.h>
#include <SimulatorLib/SimControl.h>
#include <SimulatorUi/SimIdeSimulator.h>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QGridLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QSplitter>
#include <QStringListModel>
#include <QTemporaryFile>
#include <QTreeWidget>
#include <QTreeWidgetItem>


namespace SimUi
{
	SimVduModulePage::SimVduModulePage(SimIdeSimulator* simulator, QString equipmentId, QWidget* parent) :
		SimBasePage{simulator, parent},
		m_vduEquipmentId{equipmentId}
	{
		assert(m_simulator);
		assert(m_vduEquipmentId.isEmpty() == false);

		// --
		//
		m_splitter = new QSplitter;

		m_equipmentIdLabel = new QLabel{tr("EquipmentID:"), this};
		m_equipmentIdValue = new QLabel{this};

		m_subsystemIdLabel = new QLabel{tr("SubsystemID:"), this};
		m_subsystemIdValue = new QLabel{this};

		m_channelLabel = new QLabel{tr("Channel:"), this};
		m_channelValue = new QLabel{this};

		m_moduleLabel = new QLabel{tr("Module:"), this};
		m_moduleValue = new QLabel{this};

		m_stateLine = new QFrame{this};

		m_disableButton = new QPushButton{tr("Disable"), this};
		m_stateLabel = new QLabel{this};

		m_runtimeModeLabel = new QLabel{tr("Runtime Mode:"), this};
		m_runtimeModeValue = new QLabel{tr("{}"), this};

		m_modeLine = new QFrame{this};

		m_simAppArguments = new QLabel{this};
		m_simAppArguments->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

		m_startSimAppButton = new QPushButton{tr("Start Sim App"), this};

		// m_sorLine = new QFrame{this};

		// m_tuningLine = new QFrame{this};

		/*m_armingKeyButton = new QPushButton{tr("Arming Key"), this};
		m_armingKeyStateLabel = new QLabel{tr("{0}"), this};

		m_tuningKeyButton = new QPushButton{tr("Tuning Key"), this};
		m_tuningKeyStateLabel = new QLabel{tr("{0}"), this};

		m_signalsButton = new QPushButton{tr("Signals"), this};
		m_memoryButton = new QPushButton{tr("Memory Dump"), this};
		m_codeButton = new QPushButton{tr("Code"), this};*/

		// --
		//
#if defined(Q_OS_WIN)
		QFont font = QFont("Consolas");
#else
		QFont font = QFont("Courier");
#endif
		QFont fontBold{font};
		fontBold.setBold(true);

		m_equipmentIdLabel->setFont(font);
		m_equipmentIdValue->setFont(fontBold);

		m_subsystemIdLabel->setFont(font);
		m_subsystemIdValue->setFont(font);

		m_channelLabel->setFont(font);
		m_channelValue->setFont(font);

		m_moduleLabel->setFont(font);
		m_moduleValue->setFont(font);

		m_disableButton->setFont(font);
		m_stateLabel->setFont(font);

		m_runtimeModeLabel->setFont(font);
		m_runtimeModeValue->setFont(font);

		// m_tuningModeLabel->setFont(font);
		// m_tuningModeValue->setFont(font);

		// m_armingKeyButton->setFont(font);
		// m_tuningKeyButton->setFont(font);

		// m_sorResetSwitchButton->setFont(font);
		// m_sorIsSetLabel->setFont(font);

		// m_sorSetSwitch1Button->setFont(font);
		// m_sorSetSwitch2Button->setFont(font);
		// m_sorSetSwitch3Button->setFont(font);

		m_equipmentIdValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_subsystemIdValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_channelValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_moduleValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

		// --
		//
		m_disableButton->setCheckable(true);

		// m_armingKeyButton->setCheckable(true);
		// m_tuningKeyButton->setCheckable(true);

		m_stateLine->setFrameShape(QFrame::HLine);
		m_stateLine->setFrameShadow(QFrame::Sunken);

		m_modeLine->setFrameShape(QFrame::HLine);
		m_modeLine->setFrameShadow(QFrame::Sunken);

		// m_tuningLine->setFrameShape(QFrame::HLine);
		// m_tuningLine->setFrameShadow(QFrame::Sunken);

		// m_sorLine->setFrameShape(QFrame::HLine);
		// m_sorLine->setFrameShadow(QFrame::Sunken);

		m_splitter->setChildrenCollapsible(false);

		// Left side
		//
		{
			QWidget* leftWidget = new QWidget(m_splitter);

			QGridLayout* layout = new QGridLayout;
			leftWidget->setLayout(layout);

			int row = 0;

			layout->addWidget(m_equipmentIdLabel, row, 0, 1, 1);
			layout->addWidget(m_equipmentIdValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_subsystemIdLabel, row, 0, 1, 1);
			layout->addWidget(m_subsystemIdValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_channelLabel, row, 0, 1, 1);
			layout->addWidget(m_channelValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_moduleLabel, row, 0, 1, 1);
			layout->addWidget(m_moduleValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_stateLine, row, 0, 1, 3);
			row++;

			layout->addWidget(m_disableButton, row, 0, 1, 1);
			layout->addWidget(m_stateLabel, row, 1, 1, 2);
			row++;

			layout->addWidget(m_runtimeModeLabel, row, 0, 1, 1);
			layout->addWidget(m_runtimeModeValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_modeLine, row, 0, 1, 3);
			row++;

			/*layout->addWidget(m_sorLine, row, 0, 1, 3);
			row++;

			layout->addWidget(m_tuningLine, row, 0, 1, 3);
			row++;

			layout->addWidget(m_tuningModeLabel, row, 0, 1, 1);
			layout->addWidget(m_tuningModeValue, row, 1, 1, 2);
			row++;

			layout->addWidget(m_armingKeyButton, row, 0, 1, 1);
			layout->addWidget(m_armingKeyStateLabel, row, 1, 1, 1);
			row++;

			layout->addWidget(m_tuningKeyButton, row, 0, 1, 1);
			layout->addWidget(m_tuningKeyStateLabel, row, 1, 1, 1);
			row++;*/


			// Add horizontal spacer to column 3 (0-based index)
			//
			QSpacerItem* hspacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
			layout->addItem(hspacer, 0, 3);

			// Add spacer vertical spacer
			//
			QSpacerItem* vspacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
			layout->addItem(vspacer, row++, 0, 1, 3);

			// Buttons  Signals, Memory, Code
			//
			// layout->addWidget(m_signalsButton, row, 0, 1, 1);
			// layout->addWidget(m_memoryButton, row, 1, 1, 1);
			// layout->addWidget(m_codeButton, row, 2, 1, 1);

			// Bottom section: Sim App Arguments and Start Button
			//
			layout->addWidget(m_simAppArguments, row, 0, 1, 6);
			row++;

			layout->addWidget(m_startSimAppButton, row, 0, 1, 3);
			row++;

			m_splitter->addWidget(leftWidget);
			m_splitter->setStretchFactor(0, 1);
		}

		// Right side
		//
		{
			QWidget* rightWidget = new QWidget(m_splitter);

			QGridLayout* layout = new QGridLayout;
			rightWidget->setLayout(layout);

			m_splitter->addWidget(rightWidget);
			m_splitter->setStretchFactor(1, 3);
		}

		QGridLayout* layout = new QGridLayout;
		this->setLayout(layout);
		layout->addWidget(m_splitter, 0, 0, 1, 1);

		// --
		//
		simulatorStateChanged(m_simulator->control().state());
		updateLogicModuleInfoInfo();
		updateModuleStates(m_simulator->control().status());

		// --
		//
		connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimVduModulePage::projectUpdated);
		connect(&m_simulator->control(), &Sim::Control::stateChanged, this, &SimVduModulePage::simulatorStateChanged);

		connect(m_disableButton, &QPushButton::toggled, this, &SimVduModulePage::powerOff);
		connect(m_startSimAppButton, &QPushButton::clicked, this, &SimVduModulePage::startSimApp);
		// connect(m_armingKeyButton, &QPushButton::toggled, this, &SimVduModulePage::armingKeyToggled);
		// connect(m_tuningKeyButton, &QPushButton::toggled, this, &SimVduModulePage::tuningKeyToggled);

		// connect(m_signalsButton, &QPushButton::clicked, this, &SimVduModulePage::signalsButtonClicked);
		// connect(m_codeButton, &QPushButton::clicked, this, &SimVduModulePage::codeButtonClicked);
		// connect(m_memoryButton, &QPushButton::clicked, this, &SimVduModulePage::memoryButtonClicked);

		connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimVduModulePage::updateModuleStates);

		// --
		//
		QSettings s;
		if (QByteArray ba = s.value("Simulator/SimVduModulePage/m_splitter").toByteArray(); ba.isEmpty() == false)
		{
			m_splitter->restoreState(ba);
		}

		// --
		//
		m_simAppArguments->setText(simAppArguments().join(' '));

		return;
	}

	SimVduModulePage::~SimVduModulePage()
	{
		QSettings s;

		s.setValue("Simulator/SimVduModulePage/m_splitter", m_splitter->saveState());

		return;
	}

	void SimVduModulePage::simulatorStateChanged(Sim::SimControlState state)
	{
		m_disableButton->setDisabled(state == Sim::SimControlState::Stop);

		updateModuleStates(m_simulator->control().status());

		return;
	}

	void SimVduModulePage::updateLogicModuleInfoInfo()
	{
		auto vdu = vduModule();
		if (vdu.has_value() == false)
		{
			return;
		}

		const ::LogicModuleInfo& lmInfoExtras = vdu->logicModuleExtraInfo();

		m_equipmentIdValue->setText(lmInfoExtras.equipmentID);

		m_subsystemIdValue->setText(QString("%1 (LMNumber: %2)").arg(lmInfoExtras.subsystemID).arg(lmInfoExtras.lmNumber));

		m_channelValue->setText(lmInfoExtras.subsystemChannel);

		m_moduleValue->setText(QString("%1").arg(lmInfoExtras.caption));

		return;
	}

	void SimVduModulePage::projectUpdated()
	{
		updateLogicModuleInfoInfo();
		updateModuleStates(m_simulator->control().status());
		return;
	}

	void SimVduModulePage::powerOff(bool toPowerOff)
	{
		auto module = vduModule();

		if (module.has_value() == true && module->isPowerOff() != toPowerOff)
		{
			module->setPowerOff(toPowerOff);
		}

		return;
	}

	void SimVduModulePage::startSimApp()
	{
		auto vdu = vduModule();
		if (vdu.has_value() == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot start RvduSimulatorApp: VDU module is not available."));
			return;
		}

#if defined(Q_OS_WIN)
		const QString applicationName = QStringLiteral("RvduSimulatorApp.exe");
#else
		const QString applicationName = QStringLiteral("RvduSimulatorApp");
#endif

		const QString applicationPath = QDir{QCoreApplication::applicationDirPath()}.filePath(applicationName);
		QDir buildDir{m_simulator->buildPath() + QDir::separator() + Directory::SUBSYSTEMS + QDir::separator() +
					  vdu->logicModuleExtraInfo().subsystemID};

		if (QProcess::startDetached(applicationPath, simAppArguments()) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot start RvduSimulatorApp from %1.").arg(applicationPath));
		}
	}

	// void SimVduModulePage::armingKeyToggled(bool value)
	//{
	//	if (auto module = vduModule(); module.has_value() == true && module->armingKey() != value)
	//	{
	//		module->setArmingKey(value);
	//	}

	//	return;
	//}

	// void SimVduModulePage::tuningKeyToggled(bool value)
	//{
	//	if (auto module = vduModule(); module.has_value() == true && module->tuningKey() != value)
	//	{
	//		module->setTuningKey(value);
	//	}

	//	return;
	//}

	// void SimVduModulePage::sorResetSwitchPressed()
	//{
	//	if (auto module = vduModule(); module.has_value() == true)
	//	{
	//		bool state = module->testSorResetSwitch(true);
	//		Q_UNUSED(state);
	//	}

	//	return;
	//}

	// void SimVduModulePage::sorSwitch1Toggled(bool value)
	//{
	//	if (auto module = vduModule(); module.has_value() == true && module->sorSetSwitch1() != value)
	//	{
	//		module->setSorSetSwitch1(value);
	//	}

	//	return;
	//}

	// void SimVduModulePage::sorSwitch2Toggled(bool value)
	//{
	//	if (auto module = vduModule(); module.has_value() == true && module->sorSetSwitch2() != value)
	//	{
	//		module->setSorSetSwitch2(value);
	//	}

	//	return;
	//}

	// void SimVduModulePage::sorSwitch3Toggled(bool value)
	//{
	//	if (auto module = vduModule(); module.has_value() == true && module->sorSetSwitch3() != value)
	//	{
	//		module->setSorSetSwitch3(value);
	//	}

	//	return;
	//}

	// void SimLogicModulePage::codeButtonClicked()
	//{
	//	emit openCodePageRequest(equipmentId());
	//	return;
	// }

	// void SimLogicModulePage::memoryButtonClicked()
	//{
	//	QString memoryDump = m_simulator->appSignalManager().ramDump(m_lmEquipmentId);

	//	QString fileNameTemplate = QDir::tempPath() + "/" + "ram_dump_" + m_lmEquipmentId + "_XXXXXX.txt";

	//	QTemporaryFile* file = new QTemporaryFile{fileNameTemplate, this};

	//	// The end of life of file is this object, so it will be deleted rigt after tab page is closed
	//	// thi is done to give time for QDesktopServices::openUrl(url) to open file
	//	//

	//	if (file->open() == false)
	//	{
	//		return;
	//	}

	//	QTextStream out(file);
	//	out << memoryDump;

	//	file->close();

	//	QUrl url = QUrl::fromLocalFile(file->fileName());
	//	QDesktopServices::openUrl(url);

	//	return;
	//}

	void SimVduModulePage::updateModuleStates(Sim::ControlStatus state)
	{
		{
			auto it = std::find_if(std::begin(state.m_lmDeviceModes),
								   std::end(state.m_lmDeviceModes),
								   [this](const Sim::ControlStatus::LmMode& p)
								   {
									   return p.lmEquipmentId == this->equipmentId();
								   });

			if (it != std::end(state.m_lmDeviceModes))
			{
				const Sim::ControlStatus::LmMode& lmState = *it;

				QColor color{Qt::black};

				QString text;
				text.reserve(64);

				if (state.m_state == Sim::SimControlState::Pause)
				{
					text = QObject::tr("Pause - ");
				}

				if (state.m_state != Sim::SimControlState::Stop)
				{
					switch (lmState.deviceState)
					{
					case Sim::DeviceState::Off:
						text += QStringLiteral("Off");
						break;
					case Sim::DeviceState::Start:
						text += QStringLiteral("Start");
						break;
					case Sim::DeviceState::Fault:
						text += QStringLiteral("Fault");
						color = qRgb(0xD0, 0x00, 0x00);
						break;
					case Sim::DeviceState::Operate:
						text += QStringLiteral("Operate");
						break;
					default:
						Q_ASSERT(false);
						text += QStringLiteral("Unknown");
						color = qRgb(0xD0, 0x00, 0x00);
					}
				}

				// State
				//
				QString stateText;
				if (m_simulator->isLoaded() == true)
				{
					if (m_simulator->isStopped() == true)
					{
						stateText = tr("State: Stopped");
					}
					else
					{
						stateText = text;
					}
				}

				if (stateText.isEmpty() == false)
				{
					stateText = "{" + stateText + "}";
				}

				m_stateLabel->setText(stateText);
			}
		}

		Sim::RuntimeMode runtimeMode = Sim::RuntimeMode::PoweredOffMode;
		bool disabled = false;
		// bool armingKey = false;
		// bool tuningKey = false;

		if (auto module = vduModule(); module.has_value() == true)
		{
			runtimeMode = module->runtimeMode();

			disabled = module->isPowerOff();
			// armingKey = module->armingKey();
			// tuningKey = module->tuningKey();
		}

		switch (state.m_state)
		{
		case Sim::SimControlState::Stop:
			m_disableButton->setChecked(false);
			break;
		case Sim::SimControlState::Run:
			if (m_disableButton->isChecked() != disabled)
			{
				m_disableButton->setChecked(disabled);
			}
			break;
		case Sim::SimControlState::Pause:
			if (m_disableButton->isChecked() != disabled)
			{
				m_disableButton->setChecked(disabled);
			}
			break;
		}

		{
			QString runtimeString = "{}";
			switch (runtimeMode)
			{
			case Sim::RuntimeMode::StartupMode:
				runtimeString = QStringLiteral("{StartupMode}");
				break;
			case Sim::RuntimeMode::ConfigurationMode:
				runtimeString = QStringLiteral("{ConfigurationMode}");
				break;
			case Sim::RuntimeMode::RunSafeMode:
				runtimeString = QStringLiteral("{RunSafeMode}");
				break;
			case Sim::RuntimeMode::RunMode:
				runtimeString = QStringLiteral("{RunMode}");
				break;
			case Sim::RuntimeMode::TuningMode:
				runtimeString = QStringLiteral("{TuningMode}");
				break;
			case Sim::RuntimeMode::FaultedMode:
				runtimeString = QStringLiteral("{FaultedMode}");
				break;
			case Sim::RuntimeMode::PoweredOffMode:
				runtimeString = QStringLiteral("{PoweredOffMode}");
				break;
			}

			m_runtimeModeValue->setText(runtimeString);
		}

		// Update Tuning state
		//
		// if (m_armingKeyButton->isChecked() != armingKey)
		//{
		//	m_armingKeyButton->setChecked(armingKey);
		//}

		// if (m_tuningKeyButton->isChecked() != tuningKey)
		//{
		//	m_tuningKeyButton->setChecked(tuningKey);
		// }

		// m_tuningModeValue->setText(tuningMode ? QStringLiteral("{1}") : QStringLiteral("{0}"));
		// m_armingKeyStateLabel->setText(armingKey ? QStringLiteral("{1}") : QStringLiteral("{0}"));
		// m_tuningKeyStateLabel->setText(tuningKey ? QStringLiteral("{1}") : QStringLiteral("{0}"));

		//// Update SOR Switches
		////
		// if (m_sorSetSwitch1Button->isChecked() != sorSwitch1)
		//{
		//	m_sorSetSwitch1Button->setChecked(sorSwitch1);
		// }

		// if (m_sorSetSwitch2Button->isChecked() != sorSwitch2)
		//{
		//	m_sorSetSwitch2Button->setChecked(sorSwitch2);
		// }

		// if (m_sorSetSwitch3Button->isChecked() != sorSwitch3)
		//{
		//	m_sorSetSwitch3Button->setChecked(sorSwitch3);
		// }

		// m_sorIsSetLabel->setText(tr("SOR is Set: {%1}").arg(sorIsSet ? 1 : 0));

		return;
	}

	QString SimVduModulePage::equipmentId() const
	{
		return m_vduEquipmentId;
	}

	QStringList SimVduModulePage::simAppArguments() const
	{
		auto vdu = vduModule();
		if (vdu.has_value() == false)
		{
			return {};
		}

		QStringList args;

		QDir dir{m_simulator->buildPath() + QDir::separator() + Directory::SUBSYSTEMS + QDir::separator() +
				 vdu->logicModuleExtraInfo().subsystemID};
		args << QString("--build=") + dir.path();
		args << QString("--key=%1").arg(vdu->moduleJumpers().jumpersValue);
		args << QString("--memory=%1").arg(vdu->sharedMemoryKey());

		return args;
	}

	std::optional<Sim::VduModule> SimVduModulePage::vduModule()
	{
		return m_simulator->vduModule(m_vduEquipmentId);
	}

	std::optional<Sim::VduModule> SimVduModulePage::vduModule() const
	{
		return m_simulator->vduModule(m_vduEquipmentId);
	}
} // namespace SimUi