#ifndef SIGNALLISTDIALOG_H
#define SIGNALLISTDIALOG_H

#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>
#include <QClipboard>
#include <QMutex>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const			SignalListColumn[] =
{
							QT_TRANSLATE_NOOP("SignalListDialog.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "CustomSignalID"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Chassis"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Module"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Place"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Shown on schemas"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "ADC range"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Electric range"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Electric sensor"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Physical range"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Engineering range"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Tuning"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Default value"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "Tuning range"),
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

class SignalListTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalListTable(QObject* parent = nullptr);
	virtual ~SignalListTable();

private:

	mutable QMutex			m_signalMutex;
	QVector<Metrology::Signal*> m_signalList;

	static bool				m_showADCInHex;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const;
	Metrology::Signal*		signal(int index) const;
	void					set(const QVector<Metrology::Signal*>& list_add);
	void					clear();

	QString					text(int row, int column, Metrology::Signal* pSignal) const;

	bool					showADCInHex() const { return m_showADCInHex; }
	void					setShowADCInHex(bool show) { m_showADCInHex = show; }
};

// ==============================================================================================

class SignalListDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalListDialog(bool hasButtons, QWidget *parent = nullptr);
	virtual ~SignalListDialog();

private:

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewTypeADMenu = nullptr;
	QMenu*					m_pViewTypeIOMenu = nullptr;
	QMenu*					m_pViewShowMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pExportAction = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pSignalPropertyAction = nullptr;

	QAction*				m_pTypeAnalogAction = nullptr;
	QAction*				m_pTypeDiscreteAction = nullptr;
	QAction*				m_pTypeBusAction = nullptr;
	QAction*				m_pTypeInputAction = nullptr;
	QAction*				m_pTypeInternalAction = nullptr;
	QAction*				m_pTypeOutputAction = nullptr;
	QAction*				m_pShowADCInHexAction = nullptr;

	QTableView*				m_pView = nullptr;
	SignalListTable			m_signalTable;

	QDialogButtonBox*		m_buttonBox = nullptr;

	QAction*				m_pColumnAction[SIGNAL_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	static E::SignalType	m_typeAD;
	static E::SignalInOutType m_typeIO;
	static int				m_currenIndex;

	Hash					m_selectedSignalHash = UNDEFINED_HASH;

	void					createInterface(bool hasButtons);
	void					createHeaderContexMenu();
	void					createContextMenu();

	void					updateVisibleColunm();
	void					hideColumn(int column, bool hide);

public:

	Hash					selectedSignalHash() const { return m_selectedSignalHash; }

protected:

	bool					eventFilter(QObject *object, QEvent *event);

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//
							// Signal
							//
	void					exportSignal();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pView->selectAll(); }
	void					signalProperties();


							// View
							//
	void					showTypeAnalog();
	void					showTypeDiscrete();
	void					showTypeBus();

	void					showTypeInput();
	void					showTypeInternal();
	void					showTypeOutput();

	void					showADCInHex();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&);

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // SIGNALLISTDIALOG_H
