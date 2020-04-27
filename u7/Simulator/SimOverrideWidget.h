#ifndef SIMOVERRIDEWIDGET_H
#define SIMOVERRIDEWIDGET_H

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

	virtual bool eventFilter(QObject* obj, QEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;

protected slots:
	void updateValueColumn();
	void fillListWidget(const std::vector<Sim::OverrideSignalParam>& overrideSignals);

	void selectSignal(QString appSignalId);
	void itemDoubleClicked(QTreeWidgetItem* item, int column);
	void itemChanged(QTreeWidgetItem *item, int column);

	void signalsChanged(QStringList addedAppSignalIds);
	void signalStateChanged(QStringList appSignalId);

	void clear();

	void removeSelectedSignals();
	void removeSignal(QString appSignalId);

	void addSignal();

	void setValue(QString appSignalId);
	void setValue(QString appSignalId, Sim::OverrideSignalMethod method, const QVariant& value);

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
		Index,
		CustomSignalId,
		Caption,
		Type,
		Value,
		ColumnCount
	};

	Sim::OverrideSignalParam m_overrideSignal;
};


#endif // SIMOVERRIDEWIDGET_H
