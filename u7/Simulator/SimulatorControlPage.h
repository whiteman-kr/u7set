#ifndef SIMULATORCONTROLPAGE_H
#define SIMULATORCONTROLPAGE_H

#include <memory>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QLineEdit>
#include "SimulatorBasePage.h"
#include "SimLmModel.h"

class SimulatorControlPage : public SimulatorBasePage
{
	Q_OBJECT

public:
	SimulatorControlPage(std::shared_ptr<Sim::LogicModule> logicModule,
						 std::shared_ptr<SimIdeSimulator> simulator,
						 QWidget* parent = nullptr);

protected:
	void updateLogicModuleInfoInfo();
	void fillSchemaList();

protected slots:
	void schemaFilterChanged();
	void schemaContextMenuRequested(const QPoint& pos);
	void schemaItemDoubleClicked(QTreeWidgetItem* item, int column);

	void openSelectedSchema();

signals:
	void openSchemaRequest(QString schemaId);

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
	QTreeWidget* m_schemasList = new QTreeWidget(this);
	QLineEdit* m_schemaFilterEdit = new QLineEdit(this);

	// --
	//
	std::shared_ptr<Sim::LogicModule> m_logicModule;
};

#endif // SIMULATORCONTROLPAGE_H
