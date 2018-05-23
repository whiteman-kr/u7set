#ifndef SIMOVERRIDEWIDGET_H
#define SIMOVERRIDEWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "../../Simulator/Simulator.h"

class SimOverrideWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SimOverrideWidget(Sim::Simulator* simulator, QWidget* parent = nullptr);
	virtual ~SimOverrideWidget();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

private:
	Sim::Simulator* m_simulator = nullptr;

	enum class Columns
	{
		CustomSignalId,
		Caption,
		Value,
		ColumnCount
	};

	QTreeWidget* m_signalList = nullptr;
};



#endif // SIMOVERRIDEWIDGET_H
