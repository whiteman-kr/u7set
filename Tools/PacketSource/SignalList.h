#ifndef SIGNALLIST_H
#define SIGNALLIST_H

#include <QAbstractTableModel>
#include <QColor>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <QMessageBox>

#include "SignalBase.h"

// ==============================================================================================

const char* const			SignalListColumn[] =
{
							QT_TRANSLATE_NOOP("SignalList.h", "CustomAppSignalID"),
							QT_TRANSLATE_NOOP("SignalList.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("SignalList.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("SignalList.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalList.h", "State"),
							QT_TRANSLATE_NOOP("SignalList.h", "A/D/B"),
							QT_TRANSLATE_NOOP("SignalList.h", "In/Out"),
							QT_TRANSLATE_NOOP("SignalList.h", "Eng. range"),
							QT_TRANSLATE_NOOP("SignalList.h", "Signal format"),
							QT_TRANSLATE_NOOP("SignalList.h", "State offset"),
							QT_TRANSLATE_NOOP("SignalList.h", "State bit"),
};

const int					SIGNAL_LIST_COLUMN_COUNT			= sizeof(SignalListColumn)/sizeof(SignalListColumn[0]);

const int					SIGNAL_LIST_COLUMN_CUSTOM_ID		= 0,
							SIGNAL_LIST_COLUMN_EQUIPMENT_ID		= 1,
							SIGNAL_LIST_COLUMN_APP_ID			= 2,
							SIGNAL_LIST_COLUMN_CAPTION			= 3,
							SIGNAL_LIST_COLUMN_STATE			= 4,
							SIGNAL_LIST_COLUMN_ADB				= 5,
							SIGNAL_LIST_COLUMN_INOUT			= 6,
							SIGNAL_LIST_COLUMN_EN_RANGE			= 7,
							SIGNAL_LIST_COLUMN_FORMAT			= 8,
							SIGNAL_LIST_COLUMN_STATE_OFFSET		= 9,
							SIGNAL_LIST_COLUMN_STATE_BIT		= 10;

const int					SignalListColumnWidth[SIGNAL_LIST_COLUMN_COUNT] =
{
							150, //	SIGNAL_LIST_COLUMN_CUSTOM_ID
							150, //	SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150, //	SIGNAL_LIST_COLUMN_APP_ID
							100, //	SIGNAL_LIST_COLUMN_CAPTION
							100, //	SIGNAL_LIST_COLUMN_STATE
							100, //	SIGNAL_LIST_COLUMN_ADB
							100, //	SIGNAL_LIST_COLUMN_IO
							100, //	SIGNAL_LIST_COLUMN_EN_RANGE
							100, //	SIGNAL_LIST_COLUMN_FORMAT
							100, //	SIGNAL_LIST_COLUMN_STATE_OFFSET
							100, //	SIGNAL_LIST_COLUMN_STATE_BIT
};

// ==============================================================================================

const int UPDATE_SIGNAL_STATE_TIMEOUT = 250; // 250 ms

// ==============================================================================================

class SignalTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalTable(QObject* parent = nullptr);
	virtual ~SignalTable() override;

public:

	int signalCount() const;
	PS::Signal* signalPtr(int index) const;
	void set(const std::vector<PS::Signal*> list_add);
	void clear();

	QString text(int row, int column, PS::Signal *pSignal) const;

	void updateColumn(int column);

private:

	mutable QMutex m_signalMutex;
	std::vector<PS::Signal*> m_signalList;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class SignalStateDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalStateDialog(PS::Signal* pSignal, QWidget *parent = nullptr);
	virtual ~SignalStateDialog() override;

public:

	double state() const { return m_state; }
	void setState(double state) { m_state = state; }

private:

	QLineEdit* m_stateEdit = nullptr;

	PS::Signal* m_pSignal = nullptr;

	double m_state = 0;

	void createInterface();

private slots:

	// slots of buttons
	//
	void onOk();		// for analog signal

	void onYes();		// for discrete signal
	void onNo();		// for discrete signal
};

// ==============================================================================================

#endif // SIGNALLIST_H
