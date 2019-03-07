#ifndef TESTRESULTDIALOG_H
#define TESTRESULTDIALOG_H

#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QHeaderView>
#include <QTableView>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "Options.h"


// ==============================================================================================

const char* const			TestResultColumn[] =
{
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Port"),
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Type"),
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Received packets"),
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Received bytes"),
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Skipped bytes"),
							QT_TRANSLATE_NOOP("TestResultDialog.h", "Result"),
};

const int					TEST_RESULT_COLUMN_COUNT			= sizeof(TestResultColumn)/sizeof(TestResultColumn[0]);

const int					TEST_RESULT_COLUMN_PORT				= 0,
							TEST_RESULT_COLUMN_TYPE				= 1,
							TEST_RESULT_COLUMN_PACKETS			= 2,
							TEST_RESULT_COLUMN_RECEIVED			= 3,
							TEST_RESULT_COLUMN_SKIPPED			= 4,
							TEST_RESULT_COLUMN_RESULT			= 5;

const int					TestResultColumnWidth[TEST_RESULT_COLUMN_COUNT] =
{
							100,	// TEST_RESULT_COLUMN_PORT
							100,	// TEST_RESULT_COLUMN_TYPE
							100,	// TEST_RESULT_COLUMN_PACKETS
							100,	// TEST_RESULT_COLUMN_RECEIVED
							100,	// TEST_RESULT_COLUMN_SKIPPED
							100,	// TEST_RESULT_COLUMN_RESULT
};

// ==============================================================================================

class TestResultTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit TestResultTable(QObject* parent = nullptr);
	virtual ~TestResultTable();

private:

	mutable QMutex			m_mutex;
	QList<SerialPortOption*> m_optionList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						optionCount() const;
	SerialPortOption*		option(int index) const;
	void					set(const QList<SerialPortOption*>& list_add);
	void					clear();

	QString					text(int row, int column, SerialPortOption* pOption) const;
};

// ==============================================================================================

class TestResultDialog : public QDialog
{
	Q_OBJECT

public:

	explicit TestResultDialog(QWidget *parent = nullptr);
	virtual ~TestResultDialog();

private:

	QLabel*					m_pModuleIDLabel = nullptr;
	QLineEdit*				m_pModuleIDEdit = nullptr;

	QLabel*					m_pOperatorNameLabel = nullptr;
	QLineEdit*				m_pOperatorNameEdit = nullptr;

	QTableView*				m_pView = nullptr;
	TestResultTable			m_resultTable;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();
	void					updateTestResult();

	QString					m_moduleID;
	QString					m_operatorName;

public:

	QString					moduleID() const { return m_moduleID; }
	QString					operatorName() const { return m_operatorName; }

signals:

private slots:

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // TESTRESULTDIALOG_H
