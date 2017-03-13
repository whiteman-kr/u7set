#ifndef OUTPUTSIGNALDIALOG_H
#define OUTPUTSIGNALDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const		OutputSignalColumn[] =
{
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Type"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", ""),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Rack"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Signal ID (input)"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Caption"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", ""),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Rack"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Signal ID (output)"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", "Caption"),
						QT_TRANSLATE_NOOP("OutputSignalDialog.h", ""),

};

const int				OUTPUT_SIGNAL_COLUMN_COUNT			= sizeof(OutputSignalColumn)/sizeof(OutputSignalColumn[0]);

const int				OUTPUT_SIGNAL_COLUMN_TYPE			= 0,
						OUTPUT_SIGNAL_COLUMN_SEPARATOR1		= 1,
						OUTPUT_SIGNAL_COLUMN_IN_RACK		= 2,
						OUTPUT_SIGNAL_COLUMN_IN_ID			= 3,
						OUTPUT_SIGNAL_COLUMN_IN_CAPTION		= 4,
						OUTPUT_SIGNAL_COLUMN_SEPARATOR2		= 5,
						OUTPUT_SIGNAL_COLUMN_OUT_RACK		= 6,
						OUTPUT_SIGNAL_COLUMN_OUT_ID			= 7,
						OUTPUT_SIGNAL_COLUMN_OUT_CAPTION	= 8,
						OUTPUT_SIGNAL_COLUMN_SEPARATOR3		= 9;

// ==============================================================================================

class OutputSignalTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit OutputSignalTable(QObject* parent = 0);
	~OutputSignalTable();

private:

	mutable QMutex		m_signalMutex;
	QList<OutputSignal>	m_signalList;

	static bool			m_showCustomID;

	int					columnCount(const QModelIndex &parent) const;
	int					rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant			headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant			data(const QModelIndex &index, int role) const;

public:

	int					signalCount() const;
	OutputSignal		at(int index) const;
	void				set(const QList<OutputSignal> list_add);
	void				clear();

	QString				text(int row, int column, const OutputSignal& signal) const;

	bool				showCustomID() const { return m_showCustomID; }
	void				setShowCustomID(bool show) { m_showCustomID = show; }
};

// ==============================================================================================

class OutputSignalItemDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OutputSignalItemDialog(QWidget *parent = 0);
	explicit OutputSignalItemDialog(const OutputSignal& signal, QWidget *parent = 0);
	~OutputSignalItemDialog();

private:

	QComboBox*			m_pTypeList = nullptr;

	QLineEdit*			m_pInputSignalIDEdit = nullptr;
	QLineEdit*			m_pInputSignalCaptionEdit = nullptr;
	QPushButton*		m_pInputSignalButton = nullptr;

	QLineEdit*			m_pOutputSignalIDEdit = nullptr;
	QLineEdit*			m_pOutputSignalCaptionEdit = nullptr;
	QPushButton*		m_pOutputSignalButton = nullptr;

	QCheckBox*			m_pShowCustomIDCheck = nullptr;

	QDialogButtonBox*	m_buttonBox = nullptr;

	OutputSignal		m_signal;

	static bool			m_showCustomID;

	void				createInterface();
	void				updateSignals();

public:

	OutputSignal		signal() const { return m_signal; }

signals:

private slots:

	// slots of buttons
	//
	void				selectedType(int);
	void				selectInputSignal();
	void				selectOutputSignal();

	void				showCustomID();

	void				onOk();
};

// ==============================================================================================

class OutputSignalDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OutputSignalDialog(QWidget *parent = 0);
	~OutputSignalDialog();

private:

	static int			m_columnWidth[OUTPUT_SIGNAL_COLUMN_COUNT];

	QMenuBar*			m_pMenuBar = nullptr;
	QMenu*				m_pSignalMenu = nullptr;
	QMenu*				m_pEditMenu = nullptr;
	QMenu*				m_pViewMenu = nullptr;
	QMenu*				m_pContextMenu = nullptr;

	QAction*			m_pAddAction = nullptr;
	QAction*			m_pEditAction = nullptr;
	QAction*			m_pRemoveAction = nullptr;
	QAction*			m_pImportAction = nullptr;
	QAction*			m_pExportAction = nullptr;
	QAction*			m_pFindAction = nullptr;
	QAction*			m_pCopyAction = nullptr;
	QAction*			m_pSelectAllAction = nullptr;
	QAction*			m_pShowCustomIDAction = nullptr;

	QTableView*			m_pView = nullptr;
	OutputSignalTable	m_signalTable;

	QDialogButtonBox*	m_buttonBox = nullptr;

	OutputSignalBase	m_signalBase;

	void				createInterface();
	void				createContextMenu();

signals:

private slots:

	// slots for updating
	//
	void				updateList();

	// slots of menu
	//
						// Signal
						//
	void				addSignal();
	void				editSignal();
	void				removeSignal();
	void				importSignal();
	void				exportSignal();

						// Edit
						//
	void				find();
	void				copy();
	void				selectAll() { m_pView->selectAll(); }


						// View
						//
	void				showCustomID();


	void				onContextMenu(QPoint);

	// slots for list
	//
	void				onListDoubleClicked(const QModelIndex&) { editSignal(); }

	// slots of buttons
	//
	void				onOk();
};

// ==============================================================================================

#endif // OUTPUTSIGNALDIALOG_H
