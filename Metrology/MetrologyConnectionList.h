#ifndef METROLOGYCONNECTIONDIALOG_H
#define METROLOGYCONNECTIONDIALOG_H

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

const char* const			MetrologyConnectionColumn[] =
{
							QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "Type"),
							QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "AppSignalID (source)"),
							QT_TRANSLATE_NOOP("MetrologyConnectionDialog.h", "AppSignalID (destination)"),
};

const int					METROLOGY_CONNECTION_COLUMN_COUNT			= sizeof(MetrologyConnectionColumn)/sizeof(MetrologyConnectionColumn[0]);

const int					METROLOGY_CONNECTION_COLUMN_TYPE			= 0,
							METROLOGY_CONNECTION_COLUMN_IN_ID			= 1,
							METROLOGY_CONNECTION_COLUMN_OUT_ID			= 2;

const int					MetrologyConnectionColumnWidth[METROLOGY_CONNECTION_COLUMN_COUNT] =
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

public:

	int	connectionCount() const;
	Metrology::Connection at(int index) const;
	void set(const QVector<Metrology::Connection>& list_add);
	void clear();

private:

	mutable QMutex m_connectionMutex;
	QVector<Metrology::Connection> m_connectionList;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;

	QString text(int row, int column, const Metrology::Connection& connection) const;
};

// ==============================================================================================

class MetrologyConnectionItemDialog : public QDialog
{
	Q_OBJECT

public:

	explicit MetrologyConnectionItemDialog(QWidget* parent = nullptr);
	explicit MetrologyConnectionItemDialog(const Metrology::Connection& metrologyConnection, QWidget* parent = nullptr);
	virtual ~MetrologyConnectionItemDialog();

public:

	Metrology::Connection connection() const { return m_metrologyConnection; }

private:

	QComboBox* m_pTypeList = nullptr;

	QLineEdit* m_pInputSignalIDEdit = nullptr;
	QPushButton* m_pInputSignalButton = nullptr;

	QLineEdit* m_pOutputSignalIDEdit = nullptr;
	QPushButton* m_pOutputSignalButton = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::Connection m_metrologyConnection;

	void createInterface();
	void updateSignals();

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

class MetrologyConnectionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit MetrologyConnectionDialog(QWidget* parent = nullptr);
	explicit MetrologyConnectionDialog(Metrology::Signal* pSignal, QWidget* parent = nullptr);
	virtual ~MetrologyConnectionDialog() override;

public:

	Metrology::ConnectionBase&	metrologyConnections() { return m_connectionBase; }	// metrology connections

private:

	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pConnectionMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pCreateAction = nullptr;
	QAction* m_pEditAction = nullptr;
	QAction* m_pRemoveAction = nullptr;
	QAction* m_pMoveUpAction = nullptr;
	QAction* m_pMoveDownAction = nullptr;
	QAction* m_pExportAction = nullptr;
	QAction* m_pImportAction = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QTableView* m_pView = nullptr;
	MetrologyConnectionTable m_connectionTable;

	QDialogButtonBox* m_buttonBox = nullptr;

	Metrology::ConnectionBase m_connectionBase;

	void createInterface();
	void createContextMenu();

	Metrology::Signal* m_pOutputSignal = nullptr;
	bool createConnectionBySignal(Metrology::Signal* pSignal);

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
	void moveUpConnection();
	void moveDownConnection();
	void exportConnections();
	void importConnections();

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

#endif // METROLOGYCONNECTIONDIALOG_H
