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
		void updateVduModuleInfo();

	protected slots:
		void projectUpdated();

		void powerOff(bool toPowerOff);
		void startSimApp();
		// void armingKeyToggled(bool value);
		// void tuningKeyToggled(bool value);

		// void signalsButtonClicked();
		// void codeButtonClicked();
		// void memoryButtonClicked();

		void updateModuleStates(Sim::ControlStatus state);

	public:
		QString equipmentId() const;

		QString simAppArguments() const;

	private:
		std::optional<Sim::VduModule> vduModule();
		std::optional<Sim::VduModule> vduModule() const;

	private:
		QString m_vduEquipmentId;

		// UI
		//
		QSplitter* m_splitter = nullptr;

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

		QFrame* m_modeLine = nullptr;
		// ----------------------
		QLabel* m_simAppArguments = nullptr;
		QPushButton* m_startSimAppButton = nullptr;

		// QPushButton* m_armingKeyButton = nullptr;
		// QLabel* m_armingKeyStateLabel = nullptr;

		// QPushButton* m_tuningKeyButton = nullptr;
		// QLabel* m_tuningKeyStateLabel = nullptr;

		// QPushButton* m_signalsButton = nullptr;
		// QPushButton* m_memoryButton = nullptr;
		// QPushButton* m_codeButton = nullptr;
	};
} // namespace SimUi