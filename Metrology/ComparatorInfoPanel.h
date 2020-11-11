#ifndef COMPARATORINFOPANEL_H
#define COMPARATORINFOPANEL_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>
#include <QEvent>
#include <QKeyEvent>

#include "CalibratorBase.h"
#include "SignalBase.h"
#include "Options.h"

// ==============================================================================================

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

	ComparatorInfoOption	m_comparatorInfo;
	int						m_maxComparatorCount = 0;

	mutable QMutex			m_signalMutex;
	int						m_signalCount = 0;
	QVector<IoSignalParam>	m_signalList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	void					setComparatorInfo(const ComparatorInfoOption& comparatorInfo);
	void					setMaxComparatorCount(int count);

	int						signalCount() const { return m_signalCount; }
	IoSignalParam			signalParam(int index) const;
	void					set(const QVector<IoSignalParam>& signalList);
	void					clear();

	QString					text(std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;

	void					updateState();

private slots:

	void					signalParamChanged(const QString& appSignalID);
};

// ==============================================================================================

class ComparatorInfoPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit ComparatorInfoPanel(const ComparatorInfoOption& comparatorInfo, QWidget* parent = nullptr);
	virtual ~ComparatorInfoPanel();

private:

	// elements of interface
	//
	QMainWindow*			m_pComparatorInfoWindow = nullptr;
	QTableView*				m_pView = nullptr;
	ComparatorInfoTable		m_comparatorTable;

	QMenu*					m_pContextMenu = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pComparatorPropertyAction = nullptr;

	void					createInterface();
	void					createContextMenu();

	void					hideColumn(int column, bool hide);

	QTimer*					m_updateComparatorStateTimer = nullptr;
	void					startComparatorStateTimer(int timeout);
	void					stopComparatorStateTimer();

	//
	//
	CalibratorBase*			m_pCalibratorBase = nullptr;
	ComparatorInfoOption	m_comparatorInfo;
	int						m_maxComparatorCount = 0;

	int						m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;

public:

	void					clear() { m_comparatorTable.clear(); }
	void					restartComparatorStateTimer(int timeout);

	void					setCalibratorBase(CalibratorBase* pCalibratorBase) { m_pCalibratorBase = pCalibratorBase; }
	void					setComparatorInfo(const ComparatorInfoOption& comparatorInfo);
	void					setMaxComparatorCount(int count);

protected:

	bool					eventFilter(QObject *object, QEvent *event);

public slots:

	void					signalConnectionTypeChanged(int type);

	void					activeSignalChanged(const MeasureSignal& activeSignal);		// slot informs that signal for measure was selected
	void					updateComparatorState();									// slot informs that signal for measure has updated his state

private slots:

	// slots of menu
	//
	void					copy();
	void					comparatorProperty();

	void					onContextMenu(QPoint);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) { comparatorProperty(); }
};

// ==============================================================================================

#endif // COMPARATORINFOPANEL_H
