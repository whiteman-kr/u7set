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

#include "../lib/AppSignal.h"
#include "../lib/SignalSetProvider.h"
#include "../lib/StandardColors.h"
#include "../DbLib/DbMetrologyConnection.h"

// ==============================================================================================

const char* const	MetrologyConnectionColumn[] =
{
					QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Source AppSignalID"),
					QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Connection type"),
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
	virtual ~MetrologyConnectionTable() override;

public:

	void setSignalSetProvider(SignalSetProvider* signalSetProvider);

	int	connectionCount() const;
	Metrology::Connection at(int index) const;
	void set(const QVector<Metrology::Connection>& list_add);
	void clear();

private:

	SignalSetProvider* m_signalSetProvider = nullptr;

	mutable QMutex m_connectionMutex;
	QVector<Metrology::Connection> m_connectionList;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;

	QString text(int row, int column, const Metrology::Connection& connection) const;
};

// ==============================================================================================

class DialogMetrologyConnectionItem : public QDialog
{
	Q_OBJECT

public:

	DialogMetrologyConnectionItem(SignalSetProvider* signalSetProvider, QWidget* parent = nullptr);
	virtual ~DialogMetrologyConnectionItem() override;

public:

	bool isNewConnection() { return m_isNewConnection; }

	void setConnection(bool newConnection, const Metrology::Connection& connection);

	Metrology::Connection parentConnection() const { return m_parentConnection; }
	Metrology::Connection connection() const { return m_connection; }

private:

	SignalSetProvider* m_signalSetProvider = nullptr;

	bool m_isNewConnection = false;

	QLineEdit* m_pInputSignalIDEdit = nullptr;
	QComboBox* m_pTypeList = nullptr;
	QLineEdit* m_pOutputSignalIDEdit = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::Connection m_parentConnection;
	Metrology::Connection m_connection;

	void createInterface();
	void updateSignals();

	bool electricLimitIsValid(AppSignal* pSignal);

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
	virtual ~DialogMetrologyConnection() override;

public:

	//
	//
	bool loadConnectionBase();
	bool saveConnectionBase(bool checkIn, const QString& comment);
	bool checkOutConnectionBase();

	//
	void updateList();
	void selectConnectionInList(const Metrology::Connection& connection);

	//
	//
	bool createConnectionBySignal(AppSignal* pSignal);

private:

	Metrology::DbConnectionBase m_connectionBase;
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
	QAction* m_pRestoreAction = nullptr;
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
	void enableButtonsOnToolBar();

	//
	//
	DialogMetrologyConnectionItem* m_dialogConnectionItem = nullptr;
	void fillConnection(bool newConnection, const Metrology::Connection& connection);

private:

	virtual void closeEvent(QCloseEvent * e) override;
	virtual void done(int r) override;

	void saveChanges();
	void saveColumnsWidth();
	void restoreColumnsWidth();
	void saveSettings();

protected:

	void keyPressEvent(QKeyEvent* e) override;

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
	void restoreConnection();
	void checkinConnection();
	void exportConnections();
	void importConnections();
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



