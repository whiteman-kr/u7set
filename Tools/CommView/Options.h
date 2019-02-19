#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <QtSerialPort/QSerialPort>

#include "SerialPortPacket.h"

// ==============================================================================================

const int				SERIAL_PORT_COUNT			= 5;

// ==============================================================================================

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

#define MAKEWORD(a, b)	((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define MAKEDWORD(a, b)	((DWORD)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))

// ==============================================================================================

#define					SERIALPORT_OPTIONS_REG_KEY			"Options/SerialPort/"

// ----------------------------------------------------------------------------------------------

const char* const		SerialPortParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Name"),
						QT_TRANSLATE_NOOP("Options.h", "Type"),
						QT_TRANSLATE_NOOP("Options.h", "baudRate"),
};

const int				SPO_PARAM_COUNT		= sizeof(SerialPortParam)/sizeof(SerialPortParam[0]);

const int				SPO_PARAM_NAME		= 0,
						SPO_PARAM_TYPE		= 1,
						SPO_PARAM_BAUDRATE	= 2;

// ----------------------------------------------------------------------------------------------

const char* const		RsType[] =
{
						QT_TRANSLATE_NOOP("Options.h", "RS-232"),
						QT_TRANSLATE_NOOP("Options.h", "RS-485"),
};

const int				RS_TYPE_COUNT	= sizeof(RsType)/sizeof(RsType[0]);

const int				RS_TYPE_UNKNOWN	= -1,
						RS_TYPE_232		= 0,
						RS_TYPE_485		= 1;

// ----------------------------------------------------------------------------------------------

class SerialPortOption : public QObject
{
	Q_OBJECT

public:

	explicit	SerialPortOption(QObject *parent = 0);
				SerialPortOption(const SerialPortOption& from, QObject *parent = 0);
	virtual		~SerialPortOption();

private:

	QMutex				m_dataMutex;

	bool				m_connected = false;
	bool                m_noReply = true;

	QString				m_portName;
	int					m_type = RS_TYPE_UNKNOWN;
	int					m_baudRate = 0;

	int					m_dataSize = SERIAL_PORT_HEADER_SIZE+MIN_DATA_SIZE;
	QByteArray			m_data;

	uint				m_receivedBytes = 0;
	uint				m_skippedBytes = 0;
	uint				m_queueBytes = 0;
	uint                m_packetCount = 0;

public:

	void				clear();

	bool				isConnected() const { return m_connected; }
	void				setConnected(bool connected);

	bool				isNoReply() const { return m_noReply; }
	void				setNoReply(bool noReply) { m_noReply = noReply; }

	QString				portName() const { return m_portName; }
	void				setPortName(const QString& portName) { m_portName = portName; }

	int					type() const { return m_type; }
	QString				typeStr() const;
	void				setType(int type) { m_type = type; }

	int					baudRate() const { return m_baudRate; }
	void				setBaudRate(int baudRate) { m_baudRate = baudRate; }

	int					dataSize() const { return m_dataSize; }
	QString				dataSizeStr() const;
	void				setDataSize(int size);

	SerialPortDataHeader*	dataHeader();
	quint16					data(int index);
	quint64					dataCRC();
	void				setData(const QByteArray& arr);

	int					receivedBytes() const { return m_receivedBytes; }
	QString             receivedBytesStr();
	void				setReceivedBytes(int receivedBytes) { m_receivedBytes = receivedBytes; }
	void				incReceivedBytes(int receivedBytes) { m_receivedBytes += receivedBytes; }

	int					skippedBytes() const { return m_skippedBytes; }
	QString             skippedBytesStr();
	void				setSkippedBytes(int skippedBytes) { m_skippedBytes = skippedBytes; }
	void				incSkippedBytes(int skippedBytes) { m_skippedBytes += skippedBytes; }

	int					queueBytes() const { return m_queueBytes; }
	QString             queueBytesStr();
	void				setQueueBytes(int queueBytes) { m_queueBytes = queueBytes; }

	int					packetCount() const { return m_packetCount; }
	QString             packetCountStr();
	void				setPacketCount(int count) { m_packetCount = count; }
	void				incPacketCount(int count) { m_packetCount += count; }

	bool                isDataUidOk();
	bool                isHeaderCrcOk();
	bool                isDataCrcOk();

	void				load(int index);
	void				save(int index);

	SerialPortOption&	operator=(const SerialPortOption& from);

signals:

	void				connectChanged();
};

// ==============================================================================================

#define					SERIALPORTS_OPTIONS_REG_KEY			"Options/SerialPorts/"

// ----------------------------------------------------------------------------------------------

class SerialPortsOption : public QObject
{
	Q_OBJECT

public:

	explicit	SerialPortsOption(QObject *parent = 0);
				SerialPortsOption(const SerialPortsOption& from, QObject *parent = 0);
	virtual		~SerialPortsOption();

private:

	QMutex				m_mutex;

	SerialPortOption	m_serialPort[SERIAL_PORT_COUNT];

	int					m_dataSize = 0;

public:

	void				clear();

	SerialPortOption*	port(int index);
	void				setPort(int index, const SerialPortOption& portOption);

	int					dataSize() const { return m_dataSize; }
	void				setDataSize(int size) { m_dataSize = size; }
	int					recalcDataSize();

	void				load();
	void				save();

	SerialPortsOption&	operator=(const SerialPortsOption& from);
};

// ==============================================================================================

#define					VIEW_OPTIONS_REG_KEY			"Options/View/"

// ----------------------------------------------------------------------------------------------

class ViewOption : public QObject
{
	Q_OBJECT

public:

	explicit	ViewOption(QObject *parent = 0);
				ViewOption(const ViewOption& from, QObject *parent = 0);
	virtual		~ViewOption();

private:

	bool				m_showHeader = false;
	bool				m_showInWord = false;
	bool				m_showInHex = false;
	bool				m_showInFloat = false;

public:

	void				clear();

	bool				showHeader() const { return m_showHeader; }
	void				setShowHeader(bool show) { m_showHeader = show; }

	bool				showInWord() const { return m_showInWord; }
	void				setShowInWord(bool show) { m_showInWord = show; }

	bool				showInHex() const { return m_showInHex; }
	void				setShowInHex(bool show) { m_showInHex = show; }

	bool				showInFloat() const { return m_showInFloat; }
	void				setShowInFloat(bool show) { m_showInFloat = show; }

	void				load();
	void				save();

	ViewOption&	operator=(const ViewOption& from);
};

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit	Options(QObject *parent = 0);
				Options(const Options& from, QObject *parent = 0);
	virtual		~Options();

private:

	QMutex				m_mutex;

	SerialPortsOption	m_serialPorts;
	ViewOption			m_view;

public:

	SerialPortsOption&	serialPorts() { return m_serialPorts; }
	void				setSerialPorts(const SerialPortsOption& serialPorts) { m_serialPorts = serialPorts; }

	ViewOption&			view() { return m_view; }
	void				setView(const ViewOption& view) { m_view = view; }

	void				load();
	void				save();
	void				unload();

	bool				readFromXml();

	Options&			operator=(const Options& from);
};

// ==============================================================================================

extern Options			theOptions;

// ==============================================================================================

#endif // OPTIONS_H
