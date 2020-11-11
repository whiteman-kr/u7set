#ifndef SIGNALCONNECTIONDIALOG_H
#define SIGNALCONNECTIONDIALOG_H

#include <QDebug>
#include <QScreen>
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
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Type No"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "Type"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "AppSignalID (input)"),
							QT_TRANSLATE_NOOP("SignalConnectionDialog.h", "AppSignalID (output)"),
};

const int					SIGNAL_CONNECTION_COLUMN_COUNT			= sizeof(SignalConnectionColumn)/sizeof(SignalConnectionColumn[0]);

const int					SIGNAL_CONNECTION_COLUMN_TYPE_NO		= 0,
							SIGNAL_CONNECTION_COLUMN_TYPE			= 1,
							SIGNAL_CONNECTION_COLUMN_IN_ID			= 2,
							SIGNAL_CONNECTION_COLUMN_OUT_ID			= 3;

const int					SignalConnectionColumnWidth[SIGNAL_CONNECTION_COLUMN_COUNT] =
{
							 50,	// SIGNAL_CONNECTION_COLUMN_TYPE_NO
							150,	// SIGNAL_CONNECTION_COLUMN_TYPE
							250,	// SIGNAL_CONNECTION_COLUMN_IN_ID
							250,	// SIGNAL_CONNECTION_COLUMN_OUT_ID
};

// ==============================================================================================

class SignalConnectionTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalConnectionTable(QObject* parent = nullptr);
	virtual ~SignalConnectionTable();

private:

	mutable QMutex m_connectionMutex;
	QVector<SignalConnection> m_connectionList;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;

public:

	int	connectionCount() const;
	SignalConnection at(int index) const;
	void set(const QVector<SignalConnection>& list_add);
	void clear();

	QString text(int row, int column, const SignalConnection& connection) const;
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

	QComboBox* m_pTypeList = nullptr;

	QLineEdit* m_pInputSignalIDEdit = nullptr;
	QPushButton* m_pInputSignalButton = nullptr;

	QLineEdit* m_pOutputSignalIDEdit = nullptr;
	QPushButton* m_pOutputSignalButton = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	SignalConnection m_signalConnection;

	void createInterface();
	void updateSignals();

public:

	SignalConnection connection() const { return m_signalConnection; }

private slots:

	// slots of buttons
	//
	void selectedType(int);
	void selectInputSignal();
	void selectOutputSignal();
	void selectSignal(int type);

	void onOk();
};

// ==============================================================================================

class SignalConnectionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalConnectionDialog(QWidget *parent = nullptr);
	explicit SignalConnectionDialog(Metrology::Signal* pSignal, QWidget *parent = nullptr);
	virtual ~SignalConnectionDialog() override;

private:

	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pConnectionMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pCreateAction = nullptr;
	QAction* m_pEditAction = nullptr;
	QAction* m_pRemoveAction = nullptr;
	QAction* m_pImportAction = nullptr;
	QAction* m_pExportAction = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QTableView* m_pView = nullptr;
	SignalConnectionTable m_connectionTable;

	QDialogButtonBox* m_buttonBox = nullptr;

	SignalConnectionBase m_connectionBase;

	void createInterface();
	void createContextMenu();

	Metrology::Signal*		m_pOutputSignal = nullptr;
	bool createConnectionBySignal(Metrology::Signal* pSignal);

public:

	SignalConnectionBase&	signalConnections() { return m_connectionBase; }	// signal connections

public slots:

	// slots for updating
	//
	void signalBaseLoaded();
	void updateList();

private slots:

	// slots of menu
	//
		// Signal
		//
	void createConnection();
	void editConnection();
	void removeConnection();
	void importConnections();
	void exportConnections();

		// Edit
		//
	void find();
	void copy();
	void selectAll() { m_pView->selectAll(); }

	// ContextMenu
	//
	void onContextMenu(QPoint);

	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&) { editConnection(); }

	// slots of buttons
	//
	void onOk();
};

// ==============================================================================================

#endif // SIGNALCONNECTIONDIALOG_H
