#pragma once

#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "../lib/Signal.h"
#include "../lib/SignalSetProvider.h"
#include "../lib/MetrologyConnectionBase.h"

// ==============================================================================================

const char* const	MetrologyConnectionColumn[] =
{
                    QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Type"),
                    QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "AppSignalID (source)"),
                    QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "AppSignalID (destination)"),
};

const int			METROLOGY_CONNECTION_COLUMN_COUNT		= sizeof(MetrologyConnectionColumn)/sizeof(MetrologyConnectionColumn[0]);

const int			METROLOGY_CONNECTION_COLUMN_TYPE		= 0,
					METROLOGY_CONNECTION_COLUMN_IN_ID		= 1,
					METROLOGY_CONNECTION_COLUMN_OUT_ID		= 2;

const int			ConnectionColumnWidth[METROLOGY_CONNECTION_COLUMN_COUNT] =
{
                    150,	// METROLOGY_CONNECTION_COLUMN_TYPE
                    250,	// METROLOGY_CONNECTION_COLUMN_IN_ID
                    250,	// METROLOGY_CONNECTION_COLUMN_OUT_ID
};

// ==============================================================================================

class MetrologyConnectionTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit MetrologyConnectionTable(QObject* parent = nullptr);
	virtual ~MetrologyConnectionTable();

private:

	mutable QMutex m_connectionMutex;
	QVector<Metrology::SignalConnection> m_connectionList;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;

public:

	int	connectionCount() const;
	Metrology::SignalConnection at(int index) const;
	void set(const QVector<Metrology::SignalConnection>& list_add);
	void clear();

	QString text(int row, int column, const Metrology::SignalConnection& connection) const;
};

// ==============================================================================================

class DialogMetrologyConnectionItem : public QDialog
{
	Q_OBJECT

public:

	DialogMetrologyConnectionItem(SignalSetProvider* signalSetProvider, QWidget *parent = nullptr);
	virtual ~DialogMetrologyConnectionItem() override;

private:

	SignalSetProvider* m_signalSetProvider = nullptr;

	bool m_isNewConnection = false;

	QComboBox* m_pTypeList = nullptr;
	QLineEdit* m_pInputSignalIDEdit = nullptr;
	QLineEdit* m_pOutputSignalIDEdit = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::SignalConnection m_connection;

	void createInterface();
	void updateSignals();

	bool electricLimitIsValid(Signal* pSignal);

public:

	bool isNewConnection() { return m_isNewConnection; }

	void setConnection(bool newConnection, const Metrology::SignalConnection& connection);
	Metrology::SignalConnection connection() const { return m_connection; }

private slots:

	// slots of controls
	//
	void selectedType(int);

	void onOk();
};

// ==============================================================================================

class DialogMetrologyConnection : public QDialog
{
	Q_OBJECT

public:

	DialogMetrologyConnection(SignalSetProvider* signalSetProvider, QWidget *parent = nullptr);
	virtual ~DialogMetrologyConnection() override;

private:

	const QString m_windowTitle = tr("Metrology connections");

	SignalSetProvider* m_signalSetProvider = nullptr;

	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pConnectionMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pEditAction = nullptr;
	QAction* m_pCreateAction = nullptr;
	QAction* m_pRemoveAction = nullptr;
	QAction* m_pExportAction = nullptr;
	QAction* m_pImportAction = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QString m_findText;
	QLineEdit* m_findTextEdit  = nullptr;

	QTableView* m_pView = nullptr;
	MetrologyConnectionTable m_connectionTable;

	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::ConnectionBase m_connectionBase;

	void createInterface();
	void createContextMenu();

	DialogMetrologyConnectionItem* m_dialogConnectionItem = nullptr;
	void fillConnection(bool newConnection, const Metrology::SignalConnection& connection);

protected:

	void keyPressEvent(QKeyEvent* e);

public:

	bool openConnectionBase();
	void saveConnectionBase();
	bool checkOutConnectionBase();

	void updateList();

	bool createConnectionBySignal(Signal* pSignal);

private slots:

	// slots of menu
	//
	    // menu Signal
	    //
	void editConnection();
	void newConnection();
	void connectionChanged();
	void removeConnection();
	void exportConnections();
	void importConnections();

	    // menu Edit
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



