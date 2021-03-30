#ifndef PANELCOMPARATORINFO_H
#define PANELCOMPARATORINFO_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>
#include <QEvent>
#include <QKeyEvent>

#include "CalibratorBase.h"
#include "SignalBase.h"
#include "DialogList.h"
#include "Options.h"

// ==============================================================================================

const int COMPARATOR_INFO_COLUMN_WIDTH		= 120;

// ==============================================================================================

const int COMPARATOR_INFO_UPDATE_TIMER		= 250;	//	250 ms

// ==============================================================================================

class ComparatorInfoTable : public ListTable<IoSignalParam>
{
	Q_OBJECT

public:

	explicit ComparatorInfoTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~ComparatorInfoTable() override {}

public:

	void setComparatorInfo(const ComparatorInfoOption& comparatorInfo) { m_comparatorInfo = comparatorInfo; }

	QString text(std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;

	void updateState();

private:

	ComparatorInfoOption m_comparatorInfo;

	QVariant data(const QModelIndex &index, int role) const override;

public slots:

	void signalParamChanged(const QString& appSignalID);
};

// ==============================================================================================

class PanelComparatorInfo : public QDockWidget
{
	Q_OBJECT

public:

	explicit PanelComparatorInfo(const ComparatorInfoOption& comparatorInfo, QWidget* parent = nullptr);
	virtual ~PanelComparatorInfo() override;

public:

	void clear() { m_comparatorTable.clear(); }
	void restartComparatorStateTimer(int timeout);

	void setCalibratorBase(CalibratorBase* pCalibratorBase) { m_pCalibratorBase = pCalibratorBase; }
	void setComparatorInfo(const ComparatorInfoOption& comparatorInfo);

private:

	// elements of interface
	//
	QMainWindow* m_pComparatorInfoWindow = nullptr;
	QTableView* m_pView = nullptr;
	ComparatorInfoTable m_comparatorTable;

	QMenu* m_pContextMenu = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pComparatorPropertyAction = nullptr;

	void createInterface();
	void createContextMenu();

	char* m_ptrComparatorInfoColumn[Metrology::ComparatorCount];
	char m_comparatorInfoColumn[Metrology::ComparatorCount][64];
	void hideColumn(int column, bool hide);

	QTimer* m_updateComparatorStateTimer = nullptr;
	void startComparatorStateTimer(int timeout);
	void stopComparatorStateTimer();

	//
	//
	CalibratorBase* m_pCalibratorBase = nullptr;
	ComparatorInfoOption m_comparatorInfo;

	Measure::Kind m_measureKind = Measure::Kind::NoMeasureKind;
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

protected:

	bool eventFilter(QObject* object, QEvent* event) override;

public slots:

	void measureKindChanged(Measure::Kind measureKind);
	void connectionTypeChanged(Metrology::ConnectionType connectionType);

	void activeSignalChanged(const MeasureSignal& activeSignal);		// slot informs that signal for measure was selected
	void updateComparatorState();										// slot informs that signal for measure has updated his state

private slots:

	// slots of menu
	//
	void copy();
	void comparatorProperty();

	void onContextMenu(QPoint);

	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&) { comparatorProperty(); }
};

// ==============================================================================================

#endif // PANELCOMPARATORINFO_H
