#pragma once

#include "../../Simulator/Simulator.h"
#include "../../DbLib/DbController.h"


class SimOverridePane : public QWidget, protected HasDbController
{
	Q_OBJECT

public:
	explicit SimOverridePane(Sim::Simulator* simulator, DbController* dbc, QWidget* parent = nullptr);
	virtual ~SimOverridePane();

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
	void itemChanged(QTreeWidgetItem* item, int column);

	void signalsChanged(QStringList addedAppSignalIds);
	void signalStateChanged(QStringList appSignalId);

	void clear();

	void removeSelectedSignals();
	void removeSignal(QString appSignalId);

	void addSignal();

	void saveWorkspace();
	void restoreWorkspace();

	void showSetValueDialog(QString appSignalId);
	void setValue(QString appSignalId, Sim::OverrideSignalMethod method, const QVariant& value);

private:
	Sim::Simulator* m_simulator = nullptr;

	QTreeWidget* m_treeWidget = nullptr;

	int m_currentBase = 10;											// Base for integer signals: 10, 16
	E::AnalogFormat m_currentFormat = E::AnalogFormat::g_9_or_9e;	// Current format for floating point signals
	int m_currentPrecision = -1;									// Current procision for floating point signals
};


class QOverrideListWidget : public QTreeWidget
{
	Q_OBJECT

public:
	QOverrideListWidget(Sim::Simulator* simulator, QWidget* parent);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	Sim::Simulator* m_simulator = nullptr;

	QPoint m_dragStartPos;
	QString m_dragAppSignalId;
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

