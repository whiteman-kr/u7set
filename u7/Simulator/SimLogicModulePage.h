#pragma once

#include "SimBasePage.h"
#include "SimLogicModule.h"
#include "../../VFrame30/AppSignalController.h"

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
	void updateLogicModuleInfoInfo();
	void fillSchemaList();

protected slots:
	void projectUpdated();

	void powerOff(bool toPowerOff);
	void armingKeyToggled(bool value);
	void tuningKeyToggled(bool value);
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
	std::shared_ptr<Sim::LogicModule> logicModule();
	std::shared_ptr<Sim::LogicModule> logicModule() const;

private:
	QSplitter* m_splitter = new QSplitter;

	QLabel m_equipmentIdLabel{tr("EquipmentID:"), this};
	QLabel m_equipmentIdValue{this};

	QLabel m_subsystemIdLabel{tr("SubsystemID:"), this};
	QLabel m_subsystemIdValue{this};

	QLabel m_channelLabel{tr("Channel:"), this};
	QLabel m_channelValue{this};

	QLabel m_moduleLabel{tr("Module:"), this};
	QLabel m_moduleValue{this};

	QFrame m_stateLine{this};

	QPushButton m_disableButton{tr("Disable"), this};
	QLabel m_stateLabel{this};

	QLabel m_runtimeModeLabel{tr("Runtime Mode:"), this};
	QLabel m_runtimeModeValue{tr("{}"), this};

	QFrame m_tuningLine{this};

	QLabel m_tuningModeLabel{tr("Tuning Mode:"), this};
	QLabel m_tuningModeValue{tr("{No}"), this};

	QPushButton m_armingKeyButton{tr("Arming Key"), this};
	QLabel m_armingKeyStateLabel{tr("{0}"), this};

	QPushButton m_tuningKeyButton{tr("Tuning Key"), this};
	QLabel m_tuningKeyStateLabel{tr("{0}"), this};

	QPushButton* m_signalsButton = new QPushButton{tr("Signals"), this};
	QPushButton* m_memoryButton = new QPushButton{tr("Memory Dump"), this};
	QPushButton* m_codeButton = new QPushButton{tr("Code"), this};

	QLabel* m_schemasLabel = new QLabel{tr("Schemas:"), this};
	QTreeWidget* m_schemasList = new QTreeWidget{this};

	QLineEdit* m_schemaFilterEdit = new QLineEdit{this};
	QCompleter* m_completer = new QCompleter{QStringList{}, this};

	// --
	//
	QString m_lmEquipmentId;
	VFrame30::AppSignalController* m_appSignalController = nullptr;
};

