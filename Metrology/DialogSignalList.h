#pragma once

#include "MetrologySignal.h"

#include "DialogList.h"

// ==============================================================================================

const char* const			SignalListColumn[] =
{
							QT_TRANSLATE_NOOP("DialogSignalList", "AppSignalID"),
							QT_TRANSLATE_NOOP("DialogSignalList", "CustomSignalID"),
							QT_TRANSLATE_NOOP("DialogSignalList", "EquipmentID"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Caption"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Rack"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Chassis"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Module"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Place"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Shown on schemas"),
							QT_TRANSLATE_NOOP("DialogSignalList", "ADC range"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Electric range"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Electric sensor"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Physical range"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Engineering range"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Tuning"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Default value"),
							QT_TRANSLATE_NOOP("DialogSignalList", "Tuning range"),
};

const int					SIGNAL_LIST_COLUMN_COUNT			= sizeof(SignalListColumn)/sizeof(SignalListColumn[0]);

const int					SIGNAL_LIST_COLUMN_APP_ID			= 0,
							SIGNAL_LIST_COLUMN_CUSTOM_ID		= 1,
							SIGNAL_LIST_COLUMN_EQUIPMENT_ID		= 2,
							SIGNAL_LIST_COLUMN_CAPTION			= 3,
							SIGNAL_LIST_COLUMN_RACK				= 4,
							SIGNAL_LIST_COLUMN_CHASSIS			= 5,
							SIGNAL_LIST_COLUMN_MODULE			= 6,
							SIGNAL_LIST_COLUMN_PLACE			= 7,
							SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS	= 8,
							SIGNAL_LIST_COLUMN_ADC_RANGE		= 9,
							SIGNAL_LIST_COLUMN_EL_RANGE			= 10,
							SIGNAL_LIST_COLUMN_EL_SENSOR		= 11,
							SIGNAL_LIST_COLUMN_PH_RANGE			= 12,
							SIGNAL_LIST_COLUMN_EN_RANGE			= 13,
							SIGNAL_LIST_COLUMN_TUN_SIGNAL		= 14,
							SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL	= 15,
							SIGNAL_LIST_COLUMN_TUN_RANGE		= 16;

const int					SignalListColumnWidth[SIGNAL_LIST_COLUMN_COUNT] =
{
							250,	// SIGNAL_LIST_COLUMN_APP_ID
							250,	// SIGNAL_LIST_COLUMN_CUSTOM_ID
							250,	// SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150,	// SIGNAL_LIST_COLUMN_CAPTION
							100,	// SIGNAL_LIST_COLUMN_RACK
							 60,	// SIGNAL_LIST_COLUMN_CHASSIS
							 60,	// SIGNAL_LIST_COLUMN_MODULE
							 60,	// SIGNAL_LIST_COLUMN_PLACE
							 60,	// SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS
							120,	// SIGNAL_LIST_COLUMN_ADC
							150,	// SIGNAL_LIST_COLUMN_EL_RANGE
							100,	// SIGNAL_LIST_COLUMN_EL_SENSOR
							150,	// SIGNAL_LIST_COLUMN_PH_RANGE
							150,	// SIGNAL_LIST_COLUMN_EN_RANGE
							 50,	// SIGNAL_LIST_COLUMN_TUN_SIGNAL
							100,	// SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL
							100,	// SIGNAL_LIST_COLUMN_TTUN_RANGE
};

// ==============================================================================================

class SignalListTable : public ListTable<Metrology::Signal*>
{
	Q_OBJECT

public:

	explicit SignalListTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~SignalListTable() override {}

public:

	QString					text(int row, int column, Metrology::Signal* pSignal) const;

private:

	QVariant				data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogSignalList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogSignalList(bool hasButtons, QWidget* parent = nullptr);
	virtual ~DialogSignalList() override;

public:

	Hash					selectedSignalHash() const { return m_selectedSignalHash; }

private:

	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewTypeADMenu = nullptr;
	QMenu*					m_pViewTypeIOMenu = nullptr;

	QAction*				m_pTypeAnalogAction = nullptr;
	QAction*				m_pTypeDiscreteAction = nullptr;
	QAction*				m_pTypeBusAction = nullptr;
	QAction*				m_pTypeInputAction = nullptr;
	QAction*				m_pTypeInternalAction = nullptr;
	QAction*				m_pTypeOutputAction = nullptr;

	SignalListTable			m_signalTable;

	static E::SignalType	m_typeAD;
	static E::SignalInOutType m_typeIO;

	Hash					m_selectedSignalHash = UNDEFINED_HASH;

	void					createInterface();
	void					createContextMenu();

public slots:

	void					updateVisibleColunm() override;
	void					updateList() override;	// slots for updating

private slots:

	// slots of menu
	//
	void					onProperties() override;

							// View
							//
	void					showTypeAnalog();
	void					showTypeDiscrete();
	void					showTypeBus();

	void					showTypeInput();
	void					showTypeInternal();
	void					showTypeOutput();

	// slots of buttons
	//
	void					onOk() override;
};

// ==============================================================================================
