#pragma once

#include "SimBasePage.h"

#include <Simulator/SimControlStatus.h>
#include <Simulator/SimLogicModule.h>
#include <VFrame30/AppSignalController.h>

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


	class SimLogicModulePage : public SimBasePage
	{
		Q_OBJECT

	public:
		SimLogicModulePage(SimIdeSimulator* simulator,
						   VFrame30::AppSignalController* appSignalController,
						   QString equipmentId,
						   QWidget* parent);
		virtual ~SimLogicModulePage();

	protected:
		void simulatorStateChanged(Sim::SimControlState state);
		void updateLogicModuleInfoInfo();
		void fillSchemaList();

	protected slots:
		void projectUpdated();

		void powerOff(bool toPowerOff);
		void armingKeyToggled(bool value);
		void tuningKeyToggled(bool value);

		void sorResetSwitchPressed();
		void sorSwitch1Toggled(bool value);
		void sorSwitch2Toggled(bool value);
		void sorSwitch3Toggled(bool value);

		void signalsButtonClicked();
		void codeButtonClicked();
		void memoryButtonClicked();

		void schemaFilterChanged();
		void schemaContextMenuRequested(const QPoint& pos);
		void schemaItemDoubleClicked(QTreeWidgetItem* item, int column);

		void openSelectedSchema();

		void updateFilterCompleter();

		void updateModuleStates(Sim::ControlStatus state);

	signals:
		void openSchemaRequest(QString schemaId, QStringList highlightIds);
		void openCodePageRequest(QString equipmentId);

	public:
		QString equipmentId() const;

	private:
		std::optional<Sim::LogicModule> logicModule();
		std::optional<Sim::LogicModule> logicModule() const;

	private:
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

		QPushButton* m_disableButton = nullptr;
		QLabel* m_stateLabel = nullptr;

		QLabel* m_runtimeModeLabel = nullptr;
		QLabel* m_runtimeModeValue = nullptr;

		QFrame* m_sorLine = nullptr;

		QPushButton* m_sorResetSwitchButton = nullptr;
		QLabel* m_sorIsSetLabel = nullptr;

		QPushButton* m_sorSetSwitch1Button = nullptr;
		QPushButton* m_sorSetSwitch2Button = nullptr;
		QPushButton* m_sorSetSwitch3Button = nullptr;

		QFrame* m_tuningLine = nullptr;

		QLabel* m_tuningModeLabel = nullptr;
		QLabel* m_tuningModeValue = nullptr;

		QPushButton* m_armingKeyButton = nullptr;
		QLabel* m_armingKeyStateLabel = nullptr;

		QPushButton* m_tuningKeyButton = nullptr;
		QLabel* m_tuningKeyStateLabel = nullptr;

		QPushButton* m_signalsButton = nullptr;
		QPushButton* m_memoryButton = nullptr;
		QPushButton* m_codeButton = nullptr;

		QLabel* m_schemasLabel = nullptr;
		QTreeWidget* m_schemasList = nullptr;

		QLineEdit* m_schemaFilterEdit = nullptr;
		QCompleter* m_completer = nullptr;

		// --
		//
		QString m_lmEquipmentId;
		VFrame30::AppSignalController* m_appSignalController = nullptr;
	};
} // namespace SimUi