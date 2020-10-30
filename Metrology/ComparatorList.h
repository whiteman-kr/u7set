#ifndef COMPARATORLISTDIALOG_H
#define COMPARATORLISTDIALOG_H

#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const			ComparatorListColumn[] =
{
							QT_TRANSLATE_NOOP("ComparatorList.h", "AppSignalID (Input/Internal)"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Set point"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Hysteresis"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Signal type"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Electric range"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Electric sensor"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Engineering range"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "AppSignalID (Discrete)"),
							QT_TRANSLATE_NOOP("ComparatorList.h", "Schema"),
};

const int					COMPARATOR_LIST_COLUMN_COUNT			= sizeof(ComparatorListColumn)/sizeof(ComparatorListColumn[0]);

const int					COMPARATOR_LIST_COLUMN_INPUT			= 0,
							COMPARATOR_LIST_COLUMN_SETPOINT			= 1,
							COMPARATOR_LIST_COLUMN_HYSTERESIS		= 2,
							COMPARATOR_LIST_COLUMN_TYPE				= 3,
							COMPARATOR_LIST_COLUMN_EL_RANGE			= 4,
							COMPARATOR_LIST_COLUMN_EL_SENSOR		= 5,
							COMPARATOR_LIST_COLUMN_EN_RANGE			= 6,
							COMPARATOR_LIST_COLUMN_OUTPUT			= 7,
							COMPARATOR_LIST_COLUMN_SCHEMA			= 8;


const int					ComparatorListColumnWidth[COMPARATOR_LIST_COLUMN_COUNT] =
{
							250,	// COMPARATOR_LIST_COLUMN_INPUT
							250,	// COMPARATOR_LIST_COLUMN_SETPOINT
							250,	// COMPARATOR_LIST_COLUMN_HYSTERESIS
							100,	// COMPARATOR_LIST_COLUMN_TYPE
							150,	// COMPARATOR_LIST_COLUMN_EL_RANGE
							100,	// COMPARATOR_LIST_COLUMN_EL_SENSOR
							150,	// COMPARATOR_LIST_COLUMN_EN_RANGE
							250,	// COMPARATOR_LIST_COLUMN_OUTPUT
							250,	// COMPARATOR_LIST_COLUMN_SCHEMA
};

// ==============================================================================================

class ComparatorListTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit ComparatorListTable(QObject* parent = nullptr);
	virtual ~ComparatorListTable();

private:

	mutable QMutex			m_comparatorMutex;
	QVector<std::shared_ptr<Metrology::ComparatorEx>> m_comparatorList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						comparatorCount() const;
	std::shared_ptr<Metrology::ComparatorEx> comparator(int index) const;
	void					set(const QVector<std::shared_ptr<Metrology::ComparatorEx> >& list_add);
	void					clear();

	QString					text(int row, int column, Metrology::Signal* pInSignal, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;
};

// ==============================================================================================

class ComparatorListDialog : public QDialog
{
	Q_OBJECT

public:

	explicit ComparatorListDialog(QWidget *parent = nullptr);
	virtual ~ComparatorListDialog();

private:

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pComparatorMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pExportAction = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pSignalPropertyAction = nullptr;

	QTableView*				m_pView = nullptr;
	ComparatorListTable		m_comparatorTable;

	QAction*				m_pColumnAction[COMPARATOR_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	static int				m_currenIndex;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();

	void					updateVisibleColunm();
	void					hideColumn(int column, bool hide);

protected:

	bool					eventFilter(QObject *object, QEvent *event);

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//
							// Comparator
							//
	void					exportComparator();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pView->selectAll(); }
	void					comparatorProperties();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&);
};

// ==============================================================================================

#endif // COMPARATORLISTDIALOG_H
