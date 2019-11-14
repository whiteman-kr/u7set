#ifndef SIGNALCONNECTIONDIALOG_H
#define SIGNALCONNECTIONDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QTableWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const			SignalConnectionColumn[] =
{
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Type"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", ""),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Signal ID (input)"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", ""),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Signal ID (output)"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", ""),

};

const int					SIGNAL_CONNECTION_COLUMN_COUNT			= sizeof(SignalConnectionColumn)/sizeof(SignalConnectionColumn[0]);

const int					SIGNAL_CONNECTION_COLUMN_TYPE			= 0,
							SIGNAL_CONNECTION_COLUMN_SEPARATOR1		= 1,
							SIGNAL_CONNECTION_COLUMN_IN_RACK		= 2,
							SIGNAL_CONNECTION_COLUMN_IN_ID			= 3,
							SIGNAL_CONNECTION_COLUMN_IN_CAPTION		= 4,
							SIGNAL_CONNECTION_COLUMN_SEPARATOR2		= 5,
							SIGNAL_CONNECTION_COLUMN_OUT_RACK		= 6,
							SIGNAL_CONNECTION_COLUMN_OUT_ID			= 7,
							SIGNAL_CONNECTION_COLUMN_OUT_CAPTION	= 8,
							SIGNAL_CONNECTION_COLUMN_SEPARATOR3		= 9;

const int					SignalConnectionColumnWidth[SIGNAL_CONNECTION_COLUMN_COUNT] =
{
							110,	// SIGNAL_CONNECTION_COLUMN_TYPE
							  3,	// SIGNAL_CONNECTION_COLUMN_SEPARATOR1
							100,	// SIGNAL_CONNECTION_COLUMN_IN_RACK
							250,	// SIGNAL_CONNECTION_COLUMN_IN_ID
							150,	// SIGNAL_CONNECTION_COLUMN_IN_CAPTION
							  3,	// SIGNAL_CONNECTION_COLUMN_SEPARATOR2
							100,	// SIGNAL_CONNECTION_COLUMN_OUT_RACK
							250,	// SIGNAL_CONNECTION_COLUMN_OUT_ID
							150,	// SIGNAL_CONNECTION_COLUMN_OUT_CAPTION
							  3,	// SIGNAL_CONNECTION_COLUMN_SEPARATOR3
};

// ==============================================================================================

class SignalConnectionTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalConnectionTable(QObject* parent = nullptr);
	virtual ~SignalConnectionTable();

private:

	mutable QMutex			m_connectionMutex;
	QList<SignalConnection>	m_connectionList;

	static bool				m_showCustomID;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						connectionCount() const;
	SignalConnection		at(int index) const;
	void					set(const QList<SignalConnection> list_add);
	void					clear();

	QString					text(int row, int column, const SignalConnection& signal) const;

	bool					showCustomID() const { return m_showCustomID; }
	void					setShowCustomID(bool show) { m_showCustomID = show; }
};

// ==============================================================================================

class SignalConnectionItemDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalConnectionItemDialog(QWidget *parent = nullptr);
	explicit SignalConnectionItemDialog(const SignalConnection& signalConnection, QWidget *parent = nullptr);
	virtual ~SignalConnectionItemDialog();

private:

	QComboBox*				m_pTypeList = nullptr;

	QLineEdit*				m_pInputSignalIDEdit = nullptr;
	QLineEdit*				m_pInputSignalCaptionEdit = nullptr;
	QPushButton*			m_pInputSignalButton = nullptr;

	QLineEdit*				m_pOutputSignalIDEdit = nullptr;
	QLineEdit*				m_pOutputSignalCaptionEdit = nullptr;
	QPushButton*			m_pOutputSignalButton = nullptr;

	QCheckBox*				m_pShowCustomIDCheck = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	SignalConnection		m_signalConnection;

	static bool				m_showCustomID;

	void					createInterface();
	void					updateSignals();

public:

	SignalConnection		сonnection() const { return m_signalConnection; }

signals:

private slots:

	// slots of buttons
	//
	void					selectedType(int);
	void					selectInputSignal();
	void					selectOutputSignal();

	void					showCustomID();

	void					onOk();
};

// ==============================================================================================

class SignalConnectionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalConnectionDialog(QWidget *parent = nullptr);
	virtual ~SignalConnectionDialog();

private:

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pAddAction = nullptr;
	QAction*				m_pEditAction = nullptr;
	QAction*				m_pRemoveAction = nullptr;
	QAction*				m_pImportAction = nullptr;
	QAction*				m_pExportAction = nullptr;
	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pShowCustomIDAction = nullptr;

	QTableView*				m_pView = nullptr;
	SignalConnectionTable	m_сonnectionTable;

	QDialogButtonBox*		m_buttonBox = nullptr;

	SignalConnectionBase	m_сonnectionBase;

	void					createInterface();
	void					createContextMenu();

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//
							// Signal
							//
	void					addConnection();
	void					editConnection();
	void					removeConnection();
	void					importConnections();
	void					exportConnections();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pView->selectAll(); }


							// View
							//
	void					showCustomID();


	void					onContextMenu(QPoint);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) { editConnection(); }

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // SIGNALCONNECTIONDIALOG_H
