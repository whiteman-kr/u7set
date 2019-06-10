#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include <QAbstractTableModel>
#include <QColor>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

//#include "Options.h"

#include "../../lib/Signal.h"

// ==============================================================================================

namespace PS
{
	class Signal : public ::Signal
	{

	public:

		Signal();
		Signal(const PS::Signal& from);
		virtual ~Signal();

	private:

		mutable QMutex		m_signalMutex;

		int					m_offset = -1;
		int					m_frameIndex = -1;
		int					m_frameOffset = -1;

		quint8*				m_pValueData = nullptr;

	public:

		void				clear();

		void				calcOffset();

		int					offset() const { return m_offset; }
		int					frameIndex() const { return m_frameIndex; }
		int					frameOffset() const { return m_frameOffset; }

		QString				signalTypeStr() const;
		QString				signalInOutTypeStr() const;
		QString				engeneeringRangeStr() const;
		QString				signalFormatStr() const;
		QString				stateOffsetStr() const;
		QString				stateBitStr() const;

		QString				stateStr() const;
		double				state() const;
		void				setState(double state);

		quint8*				valueData() { return m_pValueData; }
		void				setValueData(quint8* pData) { m_pValueData = pData; }

		Signal&				operator=(const Signal& from);

	signals:

	public slots:

	};
}

// ==============================================================================================

class SignalBase : public QObject
{
	Q_OBJECT

public:

	explicit SignalBase(QObject *parent = nullptr);
	virtual ~SignalBase();

private:

	mutable QMutex			m_signalMutex;
	QVector<PS::Signal>		m_signalList;
	QMap<Hash, int>			m_signalHashMap;

public:

	void					clear();
	int						count() const;

	int						readFromFile(const QString& path);

	int						append(const PS::Signal& signal);

	PS::Signal*				signalPtr(const QString& appSignalID) const;
	PS::Signal*				signalPtr(const Hash& hash) const;
	PS::Signal				signal(const Hash& hash) const;

	PS::Signal*				signalPtr(int index) const;
	PS::Signal				signal(int index) const;

	void					setSignal(int index, const PS::Signal& signal);

	SignalBase&				operator=(const SignalBase& from);

signals:

	void					signalsLoaded();

public slots:

};

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
							100, //	SIGNAL_LIST_COLUMN_CUSTOM_ID
							100, //	SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							100, //	SIGNAL_LIST_COLUMN_APP_ID
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

const int					UPDATE_SIGNAL_STATE_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class SignalTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalTable(QObject* parent = nullptr);
	virtual ~SignalTable();

private:

	mutable QMutex			m_signalMutex;
	QVector<PS::Signal*>	m_signalList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const;
	PS::Signal*				signalPtr(int index) const;
	void					set(const QVector<PS::Signal*> list_add);
	void					clear();

	QString					text(int row, int column, PS::Signal *pSignal) const;

	void					updateColumn(int column);
};

// ==============================================================================================

class SignalStateDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalStateDialog(PS::Signal* pSignal, QWidget *parent = nullptr);
	virtual ~SignalStateDialog();

private:

	QLineEdit*				m_stateEdit = nullptr;

	PS::Signal*				m_pSignal = nullptr;

	double					m_state = 0;

	void					createInterface();

public:

	double					state() const { return m_state; }
	void					setState(double state) { m_state = state; }

protected:

signals:

private slots:

	// slots of buttons
	//
	void					onOk();		// for analog signal

	void					onYes();	// for discrete signal
	void					onNo();		// for discrete signal

};

// ==============================================================================================

#endif // SIGNALBASE_H
