#ifndef DIALOGCOMPARATORLIST_H
#define DIALOGCOMPARATORLIST_H

#include "MetrologySignal.h"
#include "DialogList.h"

// ==============================================================================================

const char* const			ComparatorListColumn[] =
{
							QT_TRANSLATE_NOOP("DialogComparatorList", "Rack"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "SignalID (Input/Internal)"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Comparator No"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Set point"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Hysteresis"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Signal type"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Electric range"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Electric sensor"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Engineering range"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "SignalID (Discrete)"),
							QT_TRANSLATE_NOOP("DialogComparatorList", "Schema"),
};

const int					COMPARATOR_LIST_COLUMN_COUNT			= sizeof(ComparatorListColumn)/sizeof(ComparatorListColumn[0]);

const int					COMPARATOR_LIST_COLUMN_RACK				= 0,
							COMPARATOR_LIST_COLUMN_INPUT			= 1,
							COMPARATOR_LIST_COLUMN_CMP_NO			= 2,
							COMPARATOR_LIST_COLUMN_SETPOINT			= 3,
							COMPARATOR_LIST_COLUMN_HYSTERESIS		= 4,
							COMPARATOR_LIST_COLUMN_TYPE				= 5,
							COMPARATOR_LIST_COLUMN_EL_RANGE			= 6,
							COMPARATOR_LIST_COLUMN_EL_SENSOR		= 7,
							COMPARATOR_LIST_COLUMN_EN_RANGE			= 8,
							COMPARATOR_LIST_COLUMN_OUTPUT			= 9,
							COMPARATOR_LIST_COLUMN_SCHEMA			= 10;


const int					ComparatorListColumnWidth[COMPARATOR_LIST_COLUMN_COUNT] =
{
							100,	// COMPARATOR_LIST_COLUMN_RACK
							250,	// COMPARATOR_LIST_COLUMN_INPUT
							 50,	// COMPARATOR_LIST_COLUMN_CMP_NO
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

	void setTypeID(Metrology::SignalIDType idType) { m_idType = idType; };

	QString text(int row, int column, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;

private:

	Metrology::SignalIDType m_idType = Metrology::SignalIDType::CustomID;

	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogComparatorList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogComparatorList(QWidget* parent = nullptr);
	virtual ~DialogComparatorList() override;

private:

	QMenu* m_pComparatorMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pViewMenu = nullptr;
	QMenu* m_pViewTypeIDMenu = nullptr;

	QAction* m_pEnableMeasureAction = nullptr;
	QAction* m_pDisableMeasureAction = nullptr;
	QAction* m_pTypeIDActionList[Metrology::SignalIDTypeCount];

	ComparatorListTable m_comparatorTable;

	void createInterface();
	void createContextMenu();

	static Metrology::SignalIDType	m_idType;
	void setTypeID(Metrology::SignalIDType idType);

public slots:

	void updateVisibleColunm() override;
	void updateList() override;				// slots for updating

private slots:

	// slots of menu
	//
		// View
		//
	void showTypeID(QAction* action);

		// Edit
		//
	void onEnableMeasure();
	void onDisableMeasure();
	void onProperties() override;
};

// ==============================================================================================

#endif // DIALOGCOMPARATORLIST_H
