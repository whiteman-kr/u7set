#pragma once

#include <Simulator/Simulator.h>
#include <SimulatorUi/ISimPropertyStorage.h>

#include <QTreeWidget>


class QDragEnterEvent;
class QDropEvent;

namespace SimUi
{
	class SimOverridePane : public QWidget
	{
		Q_OBJECT

	public:
		explicit SimOverridePane(Sim::Simulator& simulator, ISimPropertyStorage& propertyStorage, QWidget* parent = nullptr);
		virtual ~SimOverridePane();

	protected:
		virtual void timerEvent(QTimerEvent* event) override;

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

		void showSetValueDialog(const QStringList& appSignalIds);
		void setValue(QString appSignalId, Sim::OverrideSignalMethod method, const QVariant& value);

	private:
		Sim::Simulator& m_simulator;
		ISimPropertyStorage& m_propertyStorage;

		QTreeWidget* m_treeWidget = nullptr;

		int m_currentBase = 10;                                       // Base for integer signals: 10, 16
		E::AnalogFormat m_currentFormat = E::AnalogFormat::g_9_or_9e; // Current format for floating point signals
		int m_currentPrecision = -1;                                  // Current precision for floating point signals

		QElapsedTimer m_signalStateSlotTimer;
	};


	class QOverrideListWidget : public QTreeWidget
	{
		Q_OBJECT

	public:
		QOverrideListWidget(Sim::Simulator& simulator, ISimPropertyStorage& propertyStorage, QWidget* parent);

	protected:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

	private:
		Sim::Simulator& m_simulator;
		ISimPropertyStorage& m_propertyStorage;

		QPoint m_dragStartPos;
		QStringList m_dragAppSignalIds;
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

		bool operator<(const QTreeWidgetItem& other) const;

		Sim::OverrideSignalParam m_overrideSignal;
	};
} // namespace SimUi