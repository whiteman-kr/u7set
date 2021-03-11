#ifndef DIALOGTUNINGSIGNALLIST_H
#define DIALOGTUNINGSIGNALLIST_H

#include "DialogList.h"
#include "TuningSignalBase.h"

// ==============================================================================================

const char* const			TuningSignalColumn[] =
{
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "Rack"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "AppSignalID"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "CustomSignalID"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "EquipmentID"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "Caption"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "State"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "Default"),
							QT_TRANSLATE_NOOP("DialogTuningSignalList", "Range"),
};

const int					TUN_SIGNAL_LIST_COLUMN_COUNT		= sizeof(TuningSignalColumn)/sizeof(TuningSignalColumn[0]);

const int					TUN_SIGNAL_LIST_COLUMN_RACK			= 0,
							TUN_SIGNAL_LIST_COLUMN_APP_ID		= 1,
							TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID	= 2,
							TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID	= 3,
							TUN_SIGNAL_LIST_COLUMN_CAPTION		= 4,
							TUN_SIGNAL_LIST_COLUMN_STATE		= 5,
							TUN_SIGNAL_LIST_COLUMN_DEFAULT		= 6,
							TUN_SIGNAL_LIST_COLUMN_RANGE		= 7;

const int					TuningSignalColumnWidth[TUN_SIGNAL_LIST_COLUMN_COUNT] =
{
							100,	 // TUN_SIGNAL_LIST_COLUMN_RACK
							250,	 // TUN_SIGNAL_LIST_COLUMN_APP_ID
							250,	 // TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID
							250,	 // TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150,	 // TUN_SIGNAL_LIST_COLUMN_CAPTION
							100,	 // TUN_SIGNAL_LIST_COLUMN_VALUE
							100,	 // TUN_SIGNAL_LIST_COLUMN_DEFAULT
							150,	 // TUN_SIGNAL_LIST_COLUMN_RANGE
};

// ==============================================================================================

class TuningSignalTable : public ListTable<Metrology::Signal*>
{
	Q_OBJECT

public:

	explicit TuningSignalTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~TuningSignalTable() override {}

public:


	QString					signalStateStr(Metrology::Signal* pSignal) const;
	QString					text(int row, int column, Metrology::Signal* pSignal) const;

private:

	QVariant				data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogTuningSignalList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogTuningSignalList(QWidget* parent = nullptr);
	virtual ~DialogTuningSignalList() override;

private:

	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewTypeADMenu = nullptr;

	QAction*				m_pSetValueAction = nullptr;

	QAction*				m_pTypeAnalogAction = nullptr;
	QAction*				m_pTypeDiscreteAction = nullptr;
	QAction*				m_pTypeBusAction = nullptr;

	TuningSignalTable		m_signalTable;

	static E::SignalType	m_typeAD;

	void					createInterface();
	void					createContextMenu();

	QTimer*					m_updateSignalStateTimer = nullptr;
	void					startSignalStateTimer();
	void					stopSignalStateTimer();

public slots:

	// slots for updating source signal list
	//
	void					updateVisibleColunm() override;
	void					updateList() override;

private slots:

	// slot informs that signal for measure has updated his state
	//
	void					updateState();

	// slots of menu
	//
							// Signal
							//
	void					onProperties() override;

							// View
							//
	void					showTypeAnalog();
	void					showTypeDiscrete();
	void					showTypeBus();
};

// ==============================================================================================

class DialogTuningSignalState : public QDialog
{
	Q_OBJECT

public:

	explicit DialogTuningSignalState(const Metrology::SignalParam& param, QWidget* parent = nullptr);
	virtual ~DialogTuningSignalState() override;

private:

	QLineEdit*				m_stateEdit = nullptr;

	Metrology::SignalParam	m_param;

	void					createInterface();

private slots:

	// slots of buttons
	//
	void					onOk();		// for analog signal

	void					onYes();	// for discrete signal
	void					onNo();		// for discrete signal

};

// ==============================================================================================

#endif // DIALOGTUNINGSIGNALLIST_H
