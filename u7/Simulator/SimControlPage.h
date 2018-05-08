#ifndef SIMULATORCONTROLPAGE_H
#define SIMULATORCONTROLPAGE_H

#include <memory>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QLineEdit>
#include "SimBasePage.h"
#include "SimLogicModule.h"

class SimControlPage : public SimBasePage
{
	Q_OBJECT

public:
	SimControlPage(SimIdeSimulator* simulator,
				   QString equipmentId,
				   QWidget* parent = nullptr);

protected:
	void updateLogicModuleInfoInfo();
	void fillSchemaList();

protected slots:
	void projectUpdated();

	void codeButtonClicked();

	void schemaFilterChanged();
	void schemaContextMenuRequested(const QPoint& pos);
	void schemaItemDoubleClicked(QTreeWidgetItem* item, int column);

	void openSelectedSchema();

signals:
	void openSchemaRequest(QString schemaId);
	void openCodePageRequest(QString equipmnetId);

public:
	QString equipmnetId() const;

private:
	std::shared_ptr<Sim::LogicModule> logicModule();
	std::shared_ptr<Sim::LogicModule> logicModule() const;

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
	QString m_lmEquipmentId;
};

#endif // SIMULATORCONTROLPAGE_H
