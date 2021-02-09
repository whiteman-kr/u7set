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
#include "../lib/StandardColors.h"

// ==============================================================================================

const char* const	MetrologyConnectionColumn[] =
{
					QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Source AppSignalID"),
					QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Type"),
					QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Destination AppSignalID"),
};

const int			METROLOGY_CONNECTION_COLUMN_COUNT		= sizeof(MetrologyConnectionColumn)/sizeof(MetrologyConnectionColumn[0]);

const int			METROLOGY_CONNECTION_COLUMN_IN_ID		= 0,
					METROLOGY_CONNECTION_COLUMN_TYPE		= 1,
					METROLOGY_CONNECTION_COLUMN_OUT_ID		= 2;

const int			ConnectionColumnWidth[METROLOGY_CONNECTION_COLUMN_COUNT] =
{
					350,	// METROLOGY_CONNECTION_COLUMN_IN_ID
					150,	// METROLOGY_CONNECTION_COLUMN_TYPE
					350,	// METROLOGY_CONNECTION_COLUMN_OUT_ID
};

// ==============================================================================================

class MetrologyConnectionTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit MetrologyConnectionTable(QObject* parent = nullptr);
	virtual ~MetrologyConnectionTable();

public:

	void setSignalSetProvider(SignalSetProvider* signalSetProvider);

	int	connectionCount() const;
	Metrology::Connection at(int index) const;
	void set(const QVector<Metrology::Connection>& list_add);
	void clear();

	QString text(int row, int column, const Metrology::Connection& connection) const;

private:

	SignalSetProvider* m_signalSetProvider = nullptr;

	mutable QMutex m_connectionMutex;
	QVector<Metrology::Connection> m_connectionList;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

class DialogMetrologyConnectionItem : public QDialog
{
	Q_OBJECT

public:

	DialogMetrologyConnectionItem(SignalSetProvider* signalSetProvider, QWidget* parent = nullptr);
	virtual ~DialogMetrologyConnectionItem();

public:

	bool isNewConnection() { return m_isNewConnection; }

	void setConnection(bool newConnection, const Metrology::Connection& connection);
	Metrology::Connection connection() const { return m_connection; }

private:

	SignalSetProvider* m_signalSetProvider = nullptr;

	bool m_isNewConnection = false;

	QLineEdit* m_pInputSignalIDEdit = nullptr;
	QComboBox* m_pTypeList = nullptr;
	QLineEdit* m_pOutputSignalIDEdit = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::Connection m_connection;

	void createInterface();
	void updateSignals();

	bool electricLimitIsValid(Signal* pSignal);

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

	DialogMetrologyConnection(SignalSetProvider* signalSetProvider, QWidget* parent = nullptr);
	virtual ~DialogMetrologyConnection();

public:

	static bool enableNewConnection(const Signal& signal);

	bool loadConnectionBase();
	void saveConnectionBase(bool checkIn, const QString& comment);
	bool checkOutConnectionBase();


	void updateList();

	bool createConnectionBySignal(Signal* pSignal);

private:

	Metrology::ConnectionBase m_connectionBase;
	bool m_isModified = false;

	//
	//
	const QString m_windowTitle = tr("Metrology connections");

	SignalSetProvider* m_signalSetProvider = nullptr;

	QMenu* m_pContextMenu = nullptr;

	QAction* m_pEditAction = nullptr;
	QAction* m_pCreateAction = nullptr;
	QAction* m_pRemoveAction = nullptr;
	QAction* m_pUnRemoveAction = nullptr;
	QAction* m_pCheckInAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pExportAction = nullptr;
	QAction* m_pImportAction = nullptr;
	QAction* m_pFindAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QString m_findText;
	QLineEdit* m_findTextEdit  = nullptr;

	QTableView* m_pView = nullptr;
	MetrologyConnectionTable m_connectionTable;

	QDialogButtonBox* m_buttonBox = nullptr;

	void createInterface();
	void createContextMenu();

	void updateCheckInStateOnToolBar();

	//
	//
	DialogMetrologyConnectionItem* m_dialogConnectionItem = nullptr;
	void fillConnection(bool newConnection, const Metrology::Connection& connection);

private:

	virtual void closeEvent(QCloseEvent * e);
	virtual void done(int r);

	void saveChanges();
	void saveColumnsWidth();
	void restoreColumnsWidth();
	void saveSettings();

protected:

	void keyPressEvent(QKeyEvent* e);

private slots:

	// slots of menu
	//
	    // menu Signal
	    //
	void editConnection();
	void newConnection();
	void connectionChanged();
	void removeConnection();
	void unremoveConnection();
	void checkinConnection();
	void exportConnections();
	void importConnections();
	void copy();

	void find();
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

class DialogComment : public QDialog
{
	Q_OBJECT

public:

	DialogComment(QWidget* parent = nullptr);

public:

	QString comment() { return m_comment; }

private:

	QString m_comment;
	QPlainTextEdit* m_pCommentEdit = nullptr;

	void createInterface();

private slots:

	void onOk();
};

// ==============================================================================================



