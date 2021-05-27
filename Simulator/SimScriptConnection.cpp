#include "SimScriptConnection.h"
#include "SimScriptSimulator.h"

namespace Sim
{
	// ----------------------------------------------------------------------------------
	//
	// ScriptConnection class implementation
	//
	// ----------------------------------------------------------------------------------

	ScriptConnection::ScriptConnection(const ScriptConnection& src) :
		m_connection(src.m_connection)
	{
	}

	ScriptConnection::ScriptConnection(std::shared_ptr<Connection> connection) :
		m_connection(connection)
	{
	}

	ScriptConnection& ScriptConnection::operator=(const ScriptConnection& src)
	{
		m_connection = src.m_connection;
		return *this;
	}

	bool ScriptConnection::isNull() const
	{
		return m_connection == nullptr;
	}

	QString ScriptConnection::connectionId() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Get connectionId error"));
			return {};
		}

		return m_connection->connectionId();
	}

	bool ScriptConnection::enabled() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return {};
		}

		return m_connection->enabled();
	}

	void ScriptConnection::setEnabled(bool value)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return;
		}

		m_connection->setEnabled(value);
		return;
	}

	bool ScriptConnection::timeout() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return {};
		}

		return m_connection->timeout();
	}

	bool ScriptConnection::enableManualSettings() const
	{
		return m_connection->connectionInfo().enableManualSettings;
	}

	bool ScriptConnection::disableDataIDControl() const
	{
		return m_connection->connectionInfo().disableDataIDControl;
	}

	QJSValue ScriptConnection::port1Info() const
	{
		return portInfo(1);
	}

	QJSValue ScriptConnection::port2Info() const
	{
		return portInfo(2);
	}

	QJSValue ScriptConnection::portInfo(int portNo) const
	{
		if (portNo != 1 && portNo != 2)
		{
			ScriptSimulator::throwScriptException(this, tr("Wrong connection portNo"));
			return {};
		}

		::ConnectionInfo ci = m_connection->connectionInfo();

		if (ci.ports.size() < portNo)
		{
			ScriptSimulator::throwScriptException(this,
								QString(tr("Port %1 is not exists in connection %2")).
									arg(portNo).arg(m_connection->connectionId()));
			return {};
		}

		QJSEngine* jsEngine = qjsEngine(this);

		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return {};
		}

		ScriptConnPortInfo* cpi = new ScriptConnPortInfo(ci.ID, ci.ports[portNo - 1]);

		return jsEngine->newQObject(cpi);
	}

	// ----------------------------------------------------------------------------------
	//
	// ScriptConnPortInfo class implementation
	//
	// ----------------------------------------------------------------------------------

	ScriptConnPortInfo::ScriptConnPortInfo(const QString& connectionID,
										   const ::ConnectionPortInfo& connPortInfo) :
		m_connectionID(connectionID),
		m_connPortInfo(connPortInfo)
	{
	}

	QString ScriptConnPortInfo::connectionID() const
	{
		return m_connectionID;
	}

	int ScriptConnPortInfo::portNo() const
	{
		return m_connPortInfo.portNo;
	}

	QString ScriptConnPortInfo::equipmentID() const
	{
		return m_connPortInfo.equipmentID;
	}

	QString ScriptConnPortInfo::moduleID() const
	{
		return m_connPortInfo.moduleID;
	}

	QString ScriptConnPortInfo::lmID() const
	{
		return m_connPortInfo.lmID;
	}

	int ScriptConnPortInfo::manualRxWordsQuantity() const
	{
		return m_connPortInfo.manualRxWordsQuantity;
	}

	int ScriptConnPortInfo::manualTxStartAddr() const
	{
		return m_connPortInfo.manualTxStartAddr;
	}

	int ScriptConnPortInfo::manualTxWordsQuantity() const
	{
		return m_connPortInfo.manualTxWordsQuantity;
	}

	bool ScriptConnPortInfo::enableSerial() const
	{
		return m_connPortInfo.enableSerial;
	}

	bool ScriptConnPortInfo::enableDuplex() const
	{
		return m_connPortInfo.enableDuplex;
	}

	QString ScriptConnPortInfo::serialMode() const
	{
		return m_connPortInfo.serialMode;
	}

	RamAddress ScriptConnPortInfo::txBufAbsAddr() const
	{
		return RamAddress(m_connPortInfo.txBufferAbsAddr, 0);
	}

	int ScriptConnPortInfo::txDataSizeW() const
	{
		return m_connPortInfo.txDataSizeW;
	}

	quint32 ScriptConnPortInfo::txDataID() const
	{
		return m_connPortInfo.txDataID;
	}

	RamAddress ScriptConnPortInfo::rxBufAbsAddr() const
	{
		return RamAddress(m_connPortInfo.rxBufferAbsAddr, 0);
	}

	int ScriptConnPortInfo::rxDataSizeW() const
	{
		return m_connPortInfo.rxDataSizeW;
	}

	quint32 ScriptConnPortInfo::rxDataID() const
	{
		return m_connPortInfo.rxDataID;
	}

	QString ScriptConnPortInfo::rxValiditySignalEquipmentID() const
	{
		return m_connPortInfo.rxValiditySignalEquipmentID;
	}

	RamAddress ScriptConnPortInfo::rxValiditySignalAbsAddr() const
	{
		return RamAddress(m_connPortInfo.rxValiditySignalAbsAddr);
	}

	bool ScriptConnPortInfo::isTxSignalExist(const QString& txAppSignalID) const
	{
		return getSignalIndex(m_connPortInfo.txSignals, txAppSignalID) != -1;
	}

	bool ScriptConnPortInfo::isRxSignalExist(const QString& rxAppSignalID) const
	{
		return getSignalIndex(m_connPortInfo.rxSignals, rxAppSignalID) != -1;
	}

	QJSValue ScriptConnPortInfo::txSignalInfo(const QString& txAppSignalID) const
	{
		int index = getSignalIndex(m_connPortInfo.txSignals, txAppSignalID);

		if (index == -1)
		{
			return {};
		}

		QJSEngine* jsEngine = qjsEngine(this);

		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return {};
		}

		ScriptConnSignalInfo* txsi = new ScriptConnSignalInfo(m_connPortInfo.txSignals[index]);

		return jsEngine->newQObject(txsi);
	}

	QJSValue ScriptConnPortInfo::rxSignalInfo(const QString& rxAppSignalID) const
	{
		int index = getSignalIndex(m_connPortInfo.rxSignals, rxAppSignalID);

		if (index == -1)
		{
			return {};
		}

		QJSEngine* jsEngine = qjsEngine(this);

		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return {};
		}

		ScriptConnSignalInfo* rxsi = new ScriptConnSignalInfo(m_connPortInfo.rxSignals[index]);

		return jsEngine->newQObject(rxsi);
	}

	int ScriptConnPortInfo::getSignalIndex(const std::vector<::ConnectionTxRxSignal>& signalsArray,
										  const QString& appSignalID) const
	{
		int index = -1;

		for(int i = 0; i < signalsArray.size(); i++)
		{
			if (signalsArray[i].IDs.contains(appSignalID) == true)
			{
				index = i;
				break;
			}
		}

		return index;
	}

	// ----------------------------------------------------------------------------------
	//
	// ScriptConnSignalInfo class implementation
	//
	// ----------------------------------------------------------------------------------

	ScriptConnSignalInfo::ScriptConnSignalInfo()
	{
	}

	ScriptConnSignalInfo::ScriptConnSignalInfo(const ::ConnectionTxRxSignal& signalInfo) :
		m_signalInfo(signalInfo)
	{
	}

	QString ScriptConnSignalInfo::appSignalID() const
	{
		return m_signalInfo.IDs.first();
	}

	QStringList ScriptConnSignalInfo::appSignalIDs() const
	{
		return m_signalInfo.IDs;
	}

	QString ScriptConnSignalInfo::signalType() const
	{
		return E::valueToString<E::SignalType>(m_signalInfo.type);
	}

	QString ScriptConnSignalInfo::analogFormat() const
	{
		return E::valueToString<E::AnalogAppSignalFormat>(m_signalInfo.analogFormat);
	}

	QString ScriptConnSignalInfo::busTypeID() const
	{
		return m_signalInfo.busTypeID;
	}

	RamAddress ScriptConnSignalInfo::addrInBuf() const
	{
		return RamAddress(m_signalInfo.addrInBuf);
	}

	RamAddress ScriptConnSignalInfo::absAddr() const
	{
		return RamAddress(m_signalInfo.absAddr);
	}

	int ScriptConnSignalInfo::dataSizeBits() const
	{
		return m_signalInfo.dataSizeBits;
	}
}
