#pragma once

#include "SimBasePage.h"

#include <SimulatorLib/SimControlStatus.h>
#include <SimulatorLib/SimVduModule.h>

#include <optional>

class QCompleter;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;

namespace SimUi
{
	class SimIdeSimulator;


	class SimVduModulePage : public SimBasePage
	{
		Q_OBJECT

	public:
		SimVduModulePage(SimIdeSimulator* simulator, QString equipmentId, QWidget* parent);
		virtual ~SimVduModulePage();

	protected:
		void simulatorStateChanged(Sim::SimControlState state);
		void updateLogicModuleInfoInfo();

	protected slots:
		void projectUpdated();

		void powerOff(bool toPowerOff);
		void startSimApp();

		void inputStateButtonToggled(size_t index, bool checked);
		void inputValidityButtonToggled(size_t index, bool checked);

		void updateInputsStates();
		void updateModuleStates(Sim::ControlStatus state);

	public:
		QString equipmentId() const;

	private:
		std::optional<Sim::VduModule> vduModule();
		std::optional<Sim::VduModule> vduModule() const;

	private:
		QString m_vduEquipmentId;

		// UI
		//
		QLabel* m_equipmentIdLabel = nullptr;
		QLabel* m_equipmentIdValue = nullptr;

		QLabel* m_subsystemIdLabel = nullptr;
		QLabel* m_subsystemIdValue = nullptr;

		QLabel* m_channelLabel = nullptr;
		QLabel* m_channelValue = nullptr;

		QLabel* m_moduleLabel = nullptr;
		QLabel* m_moduleValue = nullptr;

		QFrame* m_stateLine = nullptr;
		// ----------------------
		QPushButton* m_disableButton = nullptr;
		QLabel* m_stateLabel = nullptr;

		QLabel* m_runtimeModeLabel = nullptr;
		QLabel* m_runtimeModeValue = nullptr;

		// ----------------------

		static constexpr size_t INPUTS_COUNT = SimVduSharedData::DiscreteInputCount;
		static constexpr size_t OUTPUTS_COUNT = SimVduSharedData::DiscreteOutputCount;

		std::array<QLabel*, INPUTS_COUNT> m_inputLabels = {nullptr};
		std::array<QPushButton*, INPUTS_COUNT> m_inputStateButtons = {nullptr};
		std::array<QPushButton*, INPUTS_COUNT> m_inputValidityButtons = {nullptr};

		std::array<QLabel*, OUTPUTS_COUNT> m_outputStateLabels = {nullptr};
		std::array<QPushButton*, OUTPUTS_COUNT> m_outputStateButtons = {nullptr};

		// ----------------------
		QLabel* m_simAppArguments = nullptr;
		QPushButton* m_startSimAppButton = nullptr;
	};
} // namespace SimUi