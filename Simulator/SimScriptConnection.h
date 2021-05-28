#pragma once
#include "SimConnections.h"
#include "SimScriptRamAddress.h"

namespace Sim
{
	class Simulator;

	class ScriptConnPortInfo;
	class ScriptConnSignalInfo;

	class ScriptConnection : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString connectionID READ connectionId CONSTANT)
		Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
		Q_PROPERTY(bool timeout READ timeout CONSTANT)

		Q_PROPERTY(bool enableManualSettings READ enableManualSettings CONSTANT)
		Q_PROPERTY(bool disableDataIDControl READ disableDataIDControl CONSTANT)

		Q_PROPERTY(QJSValue port1Info READ port1Info CONSTANT)
		Q_PROPERTY(QJSValue port2Info READ port2Info CONSTANT)

	public:
		ScriptConnection() = default;
		ScriptConnection(const ScriptConnection& src);
		explicit ScriptConnection(std::shared_ptr<Connection> connection);

		ScriptConnection& operator=(const ScriptConnection& src);

	public:
		bool isNull() const;

		QString connectionId() const;

		bool enabled() const;
		void setEnabled(bool value);

		bool timeout() const;

		bool enableManualSettings() const;
		bool disableDataIDControl() const;

		QJSValue port1Info() const;
		QJSValue port2Info() const;

	public slots:
		QJSValue portInfo(int portNo) const;		// portNo is 1 or 2

	private:
		std::shared_ptr<Connection> m_connection;
	};

	class ScriptConnPortInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString connectionID READ connectionID CONSTANT)

		Q_PROPERTY(int portNo READ portNo CONSTANT)
		Q_PROPERTY(QString equipmentID READ equipmentID CONSTANT)
		Q_PROPERTY(QString moduleID READ moduleID CONSTANT)
		Q_PROPERTY(QString lmID READ lmID CONSTANT)

		Q_PROPERTY(int manualRxWordsQuantity READ manualRxWordsQuantity CONSTANT)
		Q_PROPERTY(int manualTxStartAddr READ manualTxStartAddr CONSTANT)
		Q_PROPERTY(int manualTxWordsQuantity READ manualTxWordsQuantity CONSTANT)

		Q_PROPERTY(bool enableSerial READ enableSerial CONSTANT)
		Q_PROPERTY(bool enableDuplex READ enableDuplex CONSTANT)
		Q_PROPERTY(QString serialMode READ serialMode CONSTANT)

		Q_PROPERTY(RamAddress txBufAbsAddr READ txBufAbsAddr CONSTANT)
		Q_PROPERTY(int txDataSizeW READ txDataSizeW CONSTANT)
		Q_PROPERTY(quint32 txDataID READ txDataID CONSTANT)

		Q_PROPERTY(RamAddress rxBufAbsAddr READ rxBufAbsAddr CONSTANT)
		Q_PROPERTY(int rxDataSizeW READ rxDataSizeW CONSTANT)
		Q_PROPERTY(quint32 rxDataID READ rxDataID CONSTANT)

		Q_PROPERTY(QString rxValiditySignalEquipmentID READ rxValiditySignalEquipmentID CONSTANT)
		Q_PROPERTY(RamAddress rxValiditySignalAbsAddr READ rxValiditySignalAbsAddr CONSTANT)

	public:
		ScriptConnPortInfo(const QString& connectionID,
						   const ::ConnectionPortInfo& connPortInfo);

		QString connectionID() const;
		int portNo() const;
		QString equipmentID() const;
		QString moduleID() const;
		QString lmID() const;

		int manualRxWordsQuantity() const;
		int manualTxStartAddr() const;
		int manualTxWordsQuantity() const;

		bool enableSerial() const;
		bool enableDuplex() const;
		QString serialMode() const;

		RamAddress txBufAbsAddr() const;
		int txDataSizeW() const;
		quint32 txDataID() const;

		RamAddress rxBufAbsAddr() const;
		int rxDataSizeW() const;
		quint32 rxDataID() const;

		QString rxValiditySignalEquipmentID() const;
		RamAddress rxValiditySignalAbsAddr() const;

		quint32 receivedDataID() const;
		quint32 sentDataID() const;

	public slots:
		bool isTxSignalExist(const QString& txAppSignalID) const;
		bool isRxSignalExist(const QString& rxAppSignalID) const;

		QJSValue txSignalInfo(const QString& txAppSignalID) const;
		QJSValue rxSignalInfo(const QString& rxAppSignalID) const;

	private:
		int getSignalIndex(const std::vector<::ConnectionTxRxSignal>& signalsArray, const QString& appSignalID) const;

	private:
		QString m_connectionID;
		::ConnectionPortInfo m_connPortInfo;
	};

	class ScriptConnSignalInfo : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString appSignalID READ appSignalID CONSTANT)
		Q_PROPERTY(QStringList appSignalIDs READ appSignalIDs CONSTANT)
		Q_PROPERTY(QString signalType READ signalType CONSTANT)
		Q_PROPERTY(QString analogFormat READ analogFormat CONSTANT)
		Q_PROPERTY(QString busTypeID READ busTypeID CONSTANT)
		Q_PROPERTY(RamAddress addrInBuf READ addrInBuf CONSTANT)
		Q_PROPERTY(RamAddress absAddr READ absAddr CONSTANT)
		Q_PROPERTY(int dataSizeBits READ dataSizeBits CONSTANT)

	public:
		ScriptConnSignalInfo();
		ScriptConnSignalInfo(const ::ConnectionTxRxSignal& signalInfo);

		QString appSignalID() const;
		QStringList appSignalIDs() const;
		QString signalType() const;
		QString analogFormat() const;
		QString busTypeID() const;
		RamAddress addrInBuf() const;
		RamAddress absAddr() const;
		int dataSizeBits() const;

	private:
		::ConnectionTxRxSignal m_signalInfo;
	};
}

