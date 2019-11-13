#ifndef COMPARATORINFOPANEL_H
#define COMPARATORINFOPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "SignalBase.h"

// ==============================================================================================

const int					COMPARATOR_INFO_COLUMN_COUNT		= 32;

const int					COMPARATOR_INFO_COLUMN_WIDTH		= 120;

// ==============================================================================================

const int					COMPARATOR_INFO_UPDATE_TIMER		= 250;	//	250 ms

// ==============================================================================================

class ComparatorInfoTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit ComparatorInfoTable(QObject* parent = nullptr);
	virtual ~ComparatorInfoTable();

private:

	mutable QMutex			m_signalMutex;
	int						m_signalCount = 0;
	QVector<Metrology::Signal*>	m_signalList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const { return m_signalCount; }
	Metrology::Signal*		signal(int index) const;
	void					set(const QVector<Metrology::Signal*>& signalList);
	void					clear();

	QString					text(std::shared_ptr<::Builder::Comparator> pComparator) const;

	void					updateState();
};

// ==============================================================================================

class ComparatorInfoPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit ComparatorInfoPanel(QWidget* parent = nullptr);
	virtual ~ComparatorInfoPanel();

private:

	// elements of interface
	//
	QMainWindow*			m_pComparatorInfoWindow = nullptr;
	QTableView*				m_pView = nullptr;
	ComparatorInfoTable		m_comparatorTable;

	QMenu*					m_pContextMenu = nullptr;
	QAction*				m_pComparatorPropertyAction = nullptr;

	void					createInterface();
	void					createContextMenu();

	void					hideColumn(int column, bool hide);

	QTimer*					m_updateComparatorStateTimer = nullptr;
	void					startComparatorStateTimer();
	void					stopComparatorStateTimer();

public:

	void					clear() { m_comparatorTable.clear(); }
	void					restartComparatorStateTimer();

protected:

	bool					eventFilter(QObject *object, QEvent *event);

public slots:

	void					activeSignalChanged(const MeasureSignal& activeSignal);		// slot informs that signal for measure was selected
	void					updateComparatorState();									// slot informs that signal for measure has updated his state

private slots:

	// slots of menu
	//
	void					comparatorProperty();

	void					onContextMenu(QPoint);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) { comparatorProperty(); }
};

// ==============================================================================================

#endif // COMPARATORINFOPANEL_H
