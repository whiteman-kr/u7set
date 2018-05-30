#ifndef SIMOVERRIDEWIDGET_H
#define SIMOVERRIDEWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "../../Simulator/Simulator.h"

// TO DO:
// 1. Actually SET OVERRIDE VALUE
// 2. Select signal and get its' dialog
//


class SimOverrideWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SimOverrideWidget(Sim::Simulator* simulator, QWidget* parent = nullptr);
	virtual ~SimOverrideWidget();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

	virtual void contextMenuEvent(QContextMenuEvent* event) override;

protected slots:
	void updateValueColumn();
	void fillListWidget(const std::vector<Sim::OverrideSignalParam>& overrideSignals);

	void selectSignal(QString appSignalId);
	void itemDoubleClicked(QTreeWidgetItem* item, int column);
	void itemChanged(QTreeWidgetItem *item, int column);

	void signalsChanged(QStringList addedAppSignalIds);
	void signalStateChanged(QString appSignalId);

	void clear();
	void removeSignal(QString appSignalId);
	void setValue(QString appSignalId);

private:
	Sim::Simulator* m_simulator = nullptr;

	QTreeWidget* m_treeWidget = nullptr;

	int m_currentBase = 10;											// Base for integer signals: 10, 16
	E::AnalogFormat m_currentFormat = E::AnalogFormat::g_9_or_9e;	// Current format for floating point signals
	int m_currentPrecision = -1;									// Current procision for floating point signals
};


class QOverrideTreeWidgetItem : public QTreeWidgetItem
{
public:
	QOverrideTreeWidgetItem(const Sim::OverrideSignalParam& overrideSignal);
	virtual ~QOverrideTreeWidgetItem();

	QString appSignalId() const;

public:
	enum class Columns
	{
		CustomSignalId,
		Caption,
		Type,
		Value,
		ColumnCount
	};

	Sim::OverrideSignalParam m_overrideSignal;
};


#endif // SIMOVERRIDEWIDGET_H
