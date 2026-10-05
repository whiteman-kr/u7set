#include "SimVduModulePage.h"

#include "SimWidgetPrivate.h"
#include <CommonLib/ConstStrings.h>
#include <SimulatorLib/SimControl.h>
#include <SimulatorUi/SimIdeSimulator.h>

#include <QCoreApplication>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>


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

		// m_modeLine = new QFrame{this};

		m_simAppArguments = new QLabel{this};
		m_simAppArguments->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

		m_startSimAppButton = new QPushButton{tr("Start Sim App"), this};

		// Inputs and outputs
		//
		for (size_t index = 0; index < m_inputStateButtons.size(); ++index)
		{
			m_inputLabels[index] = new QLabel{QStringLiteral("Input %1").arg(index + 1), this};

			m_inputStateButtons[index] = new QPushButton{this};
			m_inputValidityButtons[index] = new QPushButton{this};

			m_inputStateButtons[index]->setCheckable(true);
			m_inputValidityButtons[index]->setCheckable(true);

			m_inputStateButtons[index]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
			m_inputValidityButtons[index]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		}

		for (size_t i = 0; i < m_outputStateLabels.size(); ++i)
		{
			m_outputStateLabels[i] = new QLabel{QStringLiteral("Output %1").arg(i + 1), this};
			m_outputStateButtons[i] = new QPushButton{tr("{?}"), this};
			m_outputStateButtons[i]->setFlat(true);
		}

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

		m_equipmentIdValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_subsystemIdValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_channelValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_moduleValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

		// --
		//
		m_disableButton->setCheckable(true);

		m_stateLine->setFrameShape(QFrame::HLine);
		m_stateLine->setFrameShadow(QFrame::Sunken);

		// Module info column
		//
		QGroupBox* moduleGroup = new QGroupBox{tr("Module"), this};
		QFormLayout* moduleLayout = new QFormLayout{moduleGroup};
		moduleLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		moduleLayout->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
		moduleLayout->addRow(m_equipmentIdLabel, m_equipmentIdValue);
		moduleLayout->addRow(m_subsystemIdLabel, m_subsystemIdValue);
		moduleLayout->addRow(m_channelLabel, m_channelValue);
		moduleLayout->addRow(m_moduleLabel, m_moduleValue);
		moduleLayout->addRow(m_stateLine);
		moduleLayout->addRow(m_disableButton, m_stateLabel);
		moduleLayout->addRow(m_runtimeModeLabel, m_runtimeModeValue);
		moduleLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

		// Inputs column
		//
		QGroupBox* inputsGroup = new QGroupBox{tr("Inputs"), this};
		QGridLayout* inputsLayout = new QGridLayout{inputsGroup};

		for (size_t i = 0; i < m_inputLabels.size(); ++i)
		{
			inputsLayout->addWidget(m_inputLabels[i], static_cast<int>(i), 0);
			inputsLayout->addWidget(m_inputStateButtons[i], static_cast<int>(i), 1);
			inputsLayout->addWidget(m_inputValidityButtons[i], static_cast<int>(i), 2);
		}
		inputsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding),
							  static_cast<int>(m_inputLabels.size()),
							  0);

		// Outputs column
		//
		QGroupBox* outputsGroup = new QGroupBox{tr("Outputs"), this};
		QFormLayout* outputsLayout = new QFormLayout{outputsGroup};
		for (size_t index = 0; index < m_outputStateLabels.size(); index++)
		{
			outputsLayout->addRow(m_outputStateLabels[index], m_outputStateButtons[index]);
		}
		outputsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

		// Assemble three major columns
		//
		QHBoxLayout* columnsLayout = new QHBoxLayout;
		columnsLayout->addWidget(moduleGroup);
		columnsLayout->addWidget(inputsGroup);
		columnsLayout->addWidget(outputsGroup);
		columnsLayout->addStretch();

		auto startAppLayout = new QGridLayout;
		startAppLayout->addWidget(m_simAppArguments, 0, 0, 1, 4);
		startAppLayout->addWidget(m_startSimAppButton, 1, 0, 1, 1);

		// Overall page layout
		//
		QVBoxLayout* layout = new QVBoxLayout;
		layout->addLayout(columnsLayout);
		layout->addLayout(startAppLayout);

		this->setLayout(layout);

		// --
		//
		simulatorStateChanged(m_simulator->control().state());
		updateLogicModuleInfoInfo();
		updateInputsStates();
		updateModuleStates(m_simulator->control().status());

		// --
		//
		connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimVduModulePage::projectUpdated);
		connect(&m_simulator->control(), &Sim::Control::stateChanged, this, &SimVduModulePage::simulatorStateChanged);

		connect(m_disableButton, &QPushButton::toggled, this, &SimVduModulePage::powerOff);
		connect(m_startSimAppButton, &QPushButton::clicked, this, &SimVduModulePage::startSimApp);

		for (size_t i = 0; i < m_inputStateButtons.size(); ++i)
		{
			connect(m_inputStateButtons[i],
					&QPushButton::toggled,
					this,
					[this, index = i](bool toggled)
					{
						inputStateButtonToggled(index, toggled);
					});

			connect(m_inputValidityButtons[i],
					&QPushButton::toggled,
					this,
					[this, index = i](bool toggled)
					{
						inputValidityButtonToggled(index, toggled);
					});
		}

		connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimVduModulePage::updateModuleStates);

		// --
		//
		auto vdu = vduModule();
		if (vdu.has_value() == true)
		{
			m_simAppArguments->setText(vdu->simAppArguments().join(' '));
		}

		return;
	}

	SimVduModulePage::~SimVduModulePage() = default;

	void SimVduModulePage::simulatorStateChanged(Sim::SimControlState state)
	{
		m_disableButton->setDisabled(state == Sim::SimControlState::Stop);

		updateModuleStates(m_simulator->control().status());
		updateInputsStates();

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
		updateInputsStates();
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

		if (vdu->startSimAppProcess(applicationPath) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot start RvduSimulatorApp from %1.").arg(applicationPath));
		}
	}

	void SimVduModulePage::inputStateButtonToggled(size_t index, bool checked)
	{
		if (auto module = vduModule(); module.has_value() == true)
		{
			module->setDiscreteInputState(index, checked);
		}

		m_inputStateButtons[index]->setText(checked ? tr("State {1}") : tr("State {0}"));
		return;
	}

	void SimVduModulePage::inputValidityButtonToggled(size_t index, bool checked)
	{
		if (auto module = vduModule(); module.has_value() == true)
		{
			module->setDiscreteInputValidity(index, checked);
		}

		m_inputValidityButtons[index]->setText(checked ? tr("Validity {1}") : tr("Validity {0}"));
		return;
	}

	void SimVduModulePage::updateInputsStates()
	{
		auto vdu = vduModule();
		if (vdu.has_value() == false)
		{
			for (size_t index = 0; index < m_inputStateButtons.size(); ++index)
			{
				m_inputStateButtons[index]->setText(QString{"State {?}"});
				m_inputValidityButtons[index]->setText(QString{"Validity {?}"});
			}

			return;
		}

		for (size_t index = 0; index < m_inputStateButtons.size(); ++index)
		{
			bool state = vdu->discreteInputState(index);
			bool validity = vdu->discreteInputValidity(index);

			QString stateText = QString{"State {%1}"}.arg(state);
			QString validityText = QString{"Validity {%1}"}.arg(validity);

			m_inputStateButtons[index]->setText(stateText);
			m_inputValidityButtons[index]->setText(validityText);

			QSignalBlocker blockerState{m_inputStateButtons[index]};
			QSignalBlocker blockerValidity{m_inputValidityButtons[index]};

			m_inputStateButtons[index]->setChecked(state);
			m_inputValidityButtons[index]->setChecked(validity);
		}

		return;
	}

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
		std::array<std::optional<bool>, SimVduSharedData::DiscreteOutputCount> discreteOutputStates = {};


		if (auto module = vduModule(); module.has_value() == true)
		{
			runtimeMode = module->runtimeMode();
			disabled = module->isPowerOff();
			discreteOutputStates = module->discreteOutputStates();
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

		// Update Discrete Outputs
		//
		assert(m_outputStateButtons.size() == discreteOutputStates.size());

		for (size_t i = 0; i < discreteOutputStates.size(); ++i)
		{
			if (discreteOutputStates[i].has_value() == true)
			{
				m_outputStateButtons[i]->setText(discreteOutputStates[i].value() ? tr("{1}") : tr("{0}"));
			}
			else
			{
				m_outputStateButtons[i]->setText(tr("{?}"));
			}
		}

		return;
	}

	QString SimVduModulePage::equipmentId() const
	{
		return m_vduEquipmentId;
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