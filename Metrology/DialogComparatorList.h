#ifndef DIALOGCOMPARATORLIST_H
#define DIALOGCOMPARATORLIST_H

#include "MetrologySignal.h"

#include "DialogList.h"

// ==============================================================================================

const char* const			ComparatorListColumn[] =
{
							QT_TRANSLATE_NOOP("DialogComparatorList", "AppSignalID (Input/Internal)"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Set point"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Hysteresis"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Signal type"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Electric range"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Electric sensor"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Engineering range"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "AppSignalID (Discrete)"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Schema"),
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

class ComparatorListTable : public ListTable<std::shared_ptr<Metrology::ComparatorEx>>
{
	Q_OBJECT

public:

	explicit ComparatorListTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~ComparatorListTable() override {}

public:

	QString					text(int row, int column, Metrology::Signal* pInSignal, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;

private:

	QVariant				data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogComparatorList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogComparatorList(QWidget* parent = nullptr);
	virtual ~DialogComparatorList() override;

private:

	QMenu*					m_pComparatorMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;

	ComparatorListTable		m_comparatorTable;

	void					createInterface();
	void					createContextMenu();

public slots:

	void					updateVisibleColunm() override;
	void					updateList() override;				// slots for updating

private slots:

	// slots of menu
	//
	void					onProperties() override;
};

// ==============================================================================================

#endif // DIALOGCOMPARATORLIST_H
