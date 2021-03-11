#ifndef DIALOGMETROLOGYCONNECTION_H
#define DIALOGMETROLOGYCONNECTION_H

#include "../lib/MetrologySignal.h"

#include "DialogList.h"

// ==============================================================================================

const char* const			MetrologyConnectionColumn[] =
{
							QT_TRANSLATE_NOOP("DialogMetrologyConnection", "Type"),
							QT_TRANSLATE_NOOP("DialogMetrologyConnection", "AppSignalID (source)"),
							QT_TRANSLATE_NOOP("DialogMetrologyConnection", "AppSignalID (destination)"),
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

class MetrologyConnectionTable : public ListTable<Metrology::Connection>
{
	Q_OBJECT

public:

	explicit MetrologyConnectionTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~MetrologyConnectionTable() override {}

public:

	QString text(int row, int column, const Metrology::Connection& connection) const;

private:

	QVariant data(const QModelIndex &index, int role) const override;

};

// ==============================================================================================

class DialogMetrologyConnectionItem : public QDialog
{
	Q_OBJECT

public:

	explicit DialogMetrologyConnectionItem(QWidget* parent = nullptr);
	explicit DialogMetrologyConnectionItem(const Metrology::Connection& metrologyConnection, QWidget* parent = nullptr);
	virtual ~DialogMetrologyConnectionItem() override;

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

class DialogMetrologyConnection : public DialogList
{
	Q_OBJECT

public:

	explicit DialogMetrologyConnection(QWidget* parent = nullptr);
	explicit DialogMetrologyConnection(Metrology::Signal* pSignal, QWidget* parent = nullptr);
	virtual ~DialogMetrologyConnection() override;

public:

	Metrology::ConnectionBase&	metrologyConnections() { return m_connectionBase; }	// metrology connections

private:

	QMenu* m_pConnectionMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;

	QAction* m_pCreateAction = nullptr;
	QAction* m_pEditAction = nullptr;
	QAction* m_pRemoveAction = nullptr;
	QAction* m_pMoveUpAction = nullptr;
	QAction* m_pMoveDownAction = nullptr;
	QAction* m_pImportAction = nullptr;


	MetrologyConnectionTable m_connectionTable;

	Metrology::ConnectionBase m_connectionBase;

	void createInterface();
	void createContextMenu();

	Metrology::Signal* m_pOutputSignal = nullptr;
	bool createConnectionBySignal(Metrology::Signal* pSignal);

public slots:

	// slots for updating
	//
	void signalBaseLoaded();
	void updateList() override;

private slots:

	// slots of menu
	//
		// Connections
		//
	void OnNew();
	void onEdit();
	void onRremove();
	void onMoveUp();
	void onMoveDown();
	void onExport() override;
	void onImport();

	// slots for list
	//
	void onProperties() override;

};

// ==============================================================================================

#endif // DIALOGMETROLOGYCONNECTION_H
