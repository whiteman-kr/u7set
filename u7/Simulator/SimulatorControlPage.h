#ifndef SIMULATORCONTROLPAGE_H
#define SIMULATORCONTROLPAGE_H

#include <memory>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include "SimulatorBasePage.h"
#include "SimLmModel.h"

class SimulatorControlPage : public SimulatorBasePage
{
	Q_OBJECT

public:
	explicit SimulatorControlPage(std::shared_ptr<Sim::LogicModule> logicModule, QWidget* parent = nullptr);

protected:
	void updateLogicModuleInfoInfo();

public:
	QString equipmnetId() const;

private:
	QSplitter* m_splitter = new QSplitter;

	QLabel* m_subsystemIdLabel = new QLabel(this);
	QLabel* m_equipmentIdLabel = new QLabel(this);
	QLabel* m_channelLabel = new QLabel(this);

	QPushButton* m_signalsButton = new QPushButton(tr("Signals"), this);
	QPushButton* m_memoryButton = new QPushButton(tr("Memory"), this);
	QPushButton* m_codeButton = new QPushButton(tr("Code"), this);

	QLabel* m_schemasLabel = new QLabel(tr("Schemas:"), this);
	QTableWidget* m_schemasList = new QTableWidget(this);
	QLineEdit* m_schemaFilterEdit = new QLineEdit(this);

	// --
	//
	std::shared_ptr<Sim::LogicModule> m_logicModule;
};

#endif // SIMULATORCONTROLPAGE_H
