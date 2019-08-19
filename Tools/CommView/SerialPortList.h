#ifndef SERIALPORTLIST_H
#define SERIALPORTLIST_H

#include <QAbstractTableModel>
#include <QColor>

#include "Options.h"

// ==============================================================================================

const int					LIST_COLUMN_WITDH				= 100;

// ==============================================================================================

const char* const			CommStateColumn[] =
{
							QT_TRANSLATE_NOOP("SerialPortList.h", "Port"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Type"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "BaudRate"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Data size"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Received"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Skipped"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Queue"),
							QT_TRANSLATE_NOOP("SerialPortList.h", "Packets count"),

};

const int					COMM_STATE_LIST_COLUMN_COUNT	= sizeof(CommStateColumn)/sizeof(CommStateColumn[0]);

const int					COMM_STATE_LIST_COLUMN_PORT		= 0,
							COMM_STATE_LIST_COLUMN_TYPE		= 1,
							COMM_STATE_LIST_COLUMN_BAUDRATE	= 2,
							COMM_STATE_LIST_COLUMN_SIZE		= 3,
							COMM_STATE_LIST_COLUMN_RECEIVED	= 4,
							COMM_STATE_LIST_COLUMN_SKIPPED	= 5,
							COMM_STATE_LIST_COLUMN_QUEUE	= 6,
							COMM_STATE_LIST_COLUMN_PACKETS	= 7;

// ==============================================================================================

const int					UPDATE_COMM_STATE_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class CommStateTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit CommStateTable(QObject* parent = nullptr);
	virtual ~CommStateTable();

private:

	mutable QMutex				m_portMutex;
	QList<SerialPortOption*>	m_portOptionList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						portCount() const;
	SerialPortOption*		portOption(int index) const;
	void					set(const QList<SerialPortOption*> list_add);
	void					clear();

	QString					text(int row, int column, SerialPortOption* portOption) const;

	void					updateColumn(int column);
	void					updateColumns();

};

// ==============================================================================================

const int					UPDATE_COMM_HEADER_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class CommHeaderTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit CommHeaderTable(QObject* parent = nullptr);
	virtual ~CommHeaderTable();

private:

	mutable QMutex			m_dataMutex;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	QString					text(int row, int column) const;

	void					updateColumns();
};

// ==============================================================================================

const int					UPDATE_COMM_DATA_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class CommDataTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit CommDataTable(QObject* parent = nullptr);
	virtual ~CommDataTable();

private:

	mutable QMutex			m_dataMutex;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						dataSize() const;
	void					reset();

	QString					text(int row, int column) const;

	void					updateColumns();
};

// ==============================================================================================


#endif // SERIALPORTLIST_H
