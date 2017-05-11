#pragma once

#include <QObject>

#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/Address16.h"
#include "../lib/Signal.h"

#include "RawDataDescription.h"

class LogicModule;
class OutputLog;

namespace Builder
{
	class IssueLogger;
	class LmDescriptionSet;
}

namespace Hardware
{
	class DeviceModule;
	class Connection;
	class ConnectionStorage;
	class OptoModuleStorage;


	class TxRxSignal
	{
	public:
		enum Type
		{
			Raw,				// raw tx/rx signal
			Regular				// regular tx/rx signal
		};

	public:
		TxRxSignal();

		bool init(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type txRxType);

		QString appSignalID() const { return m_appSignalID; }

		bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
		bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

		bool isRaw() const { return m_type == Type::Raw; }
		bool isRegular() const { return m_type == Type::Regular; }

		Address16 addrInBuf() const { return m_addrInBuf; }
		void setAddrInBuf(Address16& addr);

		int sizeB() const { return m_sizeB; }

	private:
		Type m_type = Type::Regular;

		QString m_appSignalID;
		E::SignalType m_signalType = E::SignalType::Analog;
		int m_sizeB = 0;					// signal size in tx(rx)Buffer in bits

		Address16 m_addrInBuf;				// relative signal address from beginning of txBuffer (rxBuffer)
	};

	typedef std::shared_ptr<TxRxSignal> TxRxSignalShared;


	class OptoPort : public QObject
	{
		Q_OBJECT

	public:
		enum class Mode
		{
			Optical,
			Serial
		};
		Q_ENUM(Mode)

		enum class SerialMode
		{
			RS232,
			RS485
		};
		Q_ENUM(SerialMode)

		const quint16 NOT_USED_PORT_ID = 0;

		static const int TX_DATA_ID_SIZE_W = sizeof(quint32) / sizeof(quint16);		// size of opto port's txDataID in words

	public:
		OptoPort();

		bool init(const DeviceController* controller, int portNo, Builder::IssueLogger* log);

		bool addRawTxSignals(const HashedVector<QString, Signal *>& lmAssociatedSignals);
		bool addSerialRawRxSignals(const HashedVector<QString, Signal *>& lmAssociatedSignals);
		bool addSerialRegularRxSignal(const Signal* appSignal);

		bool appendRegularTxSignal(const Signal* s);
		bool sortTxSignals();
		bool sortSerialRxSignals();
		bool calculateTxSignalsAddresses();
		bool calculateTxDataID();
		bool calculateRxDataID();

		void getTxSignals(QVector<TxRxSignalShared>& txSignals) const;
		void getRxSignals(QVector<TxRxSignalShared>& rxSignals) const;

		void getTxAnalogSignals(QVector<TxRxSignalShared>& txSignals) const;
		void getTxDiscreteSignals(QVector<TxRxSignalShared>& txSignals) const;

		bool isTxSignalExists(const QString& appSignalID);
		bool isUsedInConnection() const;

		bool isOptical() const { return m_mode ==  Mode::Optical; }
		bool isSerial() const { return m_mode ==  Mode::Serial; }

		Address16 getTxSignalAddrInBuf(const QString& appSignalID) const;
		Address16 getTxSignalAbsAddr(const QString& appSignalID) const;

		bool parseRawDescription();
		bool calculatePortRawDataSize(OptoModuleStorage* optoStorage);

		bool getSignalRxAddressSerial(std::shared_ptr<Connection> connection,
										const QString& appSignalID,
										QUuid receiverUuid,
										SignalAddress16 &addr);

		Q_INVOKABLE quint16 portID() const { return m_portID; }
		void setPortID(quint16 portID) { m_portID = portID; }

		Q_INVOKABLE quint32 txDataID() const { return m_txDataID; }

		Q_INVOKABLE QString equipmentID() const { return m_equipmentID; }

		Q_INVOKABLE QString linkedPortID() const { return m_linkedPortID; }
		void setLinkedPortID(const QString& linkedPortID) { m_linkedPortID = linkedPortID; }

		Q_INVOKABLE bool isLinked() const { return !m_linkedPortID.isEmpty(); }

		Q_INVOKABLE QString connectionID() const { return m_connectionID; }
		void setConnectionID(const QString& connectionID) { m_connectionID = connectionID; }

		Q_INVOKABLE Mode mode() const { return m_mode; }
		void setMode(Mode mode) { m_mode = mode; }

		Q_INVOKABLE SerialMode serialMode() const { return m_serialMode; }
		void setSerialMode(SerialMode serialMode) { m_serialMode = serialMode; }

		int rxStartAddress() const { return m_rxAddress; }
		void setRxStartAddress(int address) { m_rxAddress = address; }

		Q_INVOKABLE int txStartAddress() const { return m_txStartAddress; }
		void setTxStartAddress(int address) { m_txStartAddress = address; }

		int absTxStartAddress() { return m_absTxStartAddress; }
		void setAbsTxStartAddress(int address) { m_absTxStartAddress = address; }

		Q_INVOKABLE bool enableDuplex() const { return m_enableDuplex; }
		void setEnableDuplex(bool enable) { m_enableDuplex = enable; }

		const DeviceController* deviceController() const { return m_controller; }

		QString optoModuleID() const { return m_optoModuleID; }

		QString rawDataDescriptionStr() const { return m_rawDataDescriptionStr; }
		void setRawDataDescriptionStr(const QString& description) { m_rawDataDescriptionStr = description; }

		const RawDataDescription& rawDataDescription() const { return m_rawDataDescription; }

		int txRawDataSizeW() const { return m_txRawDataSizeW; }
		void setTxRawDataSizeW(int rawDataSizeW);

		bool hasTxRawData() const { return m_rawDataDescriptionStr.isEmpty() == false; }

		int txAnalogSignalsSizeW() const { return m_txAnalogSignalsSizeW; }
		int txAnalogSignalsCount() const { return m_txAnalogSignalsCount; }

		int txDiscreteSignalsSizeW() const { return m_txDiscreteSignalsSizeW; }
		int txDiscreteSignalsCount() const { return m_txDiscreteSignalsCount; }

		int portNo() const { return m_portNo; }

		bool txRawDataSizeWIsCalculated() const { return m_txRawDataSizeWIsCalculated; }

		Q_INVOKABLE int txDataSizeW() const { return m_txDataSizeW; }

		Q_INVOKABLE int rxDataSizeW() const { return m_rxDataSizeW; }
		void setRxDataSizeW(int rxDataSizeW) { m_rxDataSizeW = rxDataSizeW; }

		Q_INVOKABLE bool manualSettings() const { return m_manualSettings; }
		void setManualSettings(bool manualSettings) { m_manualSettings = manualSettings; }

		int manualTxStartAddressW() const { return m_manualTxStartAddressW; }
		void setManualTxStartAddressW(int manualTxStartAddressW) { m_manualTxStartAddressW = manualTxStartAddressW; }

		int manualTxSizeW() const { return m_manualTxSizeW; }
		void setManualTxSizeW(int manualTxSizeW) { m_manualTxSizeW = manualTxSizeW; }

		int manualRxSizeW() const { return m_manualRxSizeW; }
		void setManualRxSizeW(int manualRxSizeW) { m_manualRxSizeW = manualRxSizeW; }

	private:
		bool appendTxSignal(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type type);
		bool appendRxSignal(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type type);

		bool appendRawTxRxSignals();

		void sortByOffsetBitNoAscending(HashedVector<QString, TxRxSignalShared>& signalList, int startIndex, int count);
		void sortByAppSignalIdAscending(HashedVector<QString, TxRxSignalShared>& signalList, int startIndex, int count);

		bool checkSignalsOffsets(const HashedVector<QString, TxRxSignalShared>& signalList, int startIndex, int count);

		bool sortTxRxSignalList(HashedVector<QString, TxRxSignalShared>& signalList);

	private:
		QString m_equipmentID;
		int m_portNo = 0;

		Mode m_mode = Mode::Optical;
		SerialMode m_serialMode = SerialMode::RS232;

		bool m_enableDuplex = false;    // serial mode and OCMN only

		bool m_manualSettings = false;
		int m_manualTxStartAddressW = 0;
		int m_manualTxSizeW = 0;
		int m_manualRxSizeW = 0;

		QString m_rawDataDescriptionStr;
		RawDataDescription m_rawDataDescription;

		QString m_lmID;
		const DeviceController* m_controller = nullptr;

		QString m_optoModuleID;
		QString m_linkedPortID;
		QString m_connectionID;

		quint16 m_portID = NOT_USED_PORT_ID;			// range 0..999, 0 - not used port ID, 1..999 - linked port ID

		Builder::IssueLogger* m_log = nullptr;

		//

		int m_txStartAddress = 0;						// address of port's Tx data relative from opto module appDataOffset
		int m_absTxStartAddress = 0;					// absoulte address of port Tx data in LM memory
		int m_txDataSizeW = 0;							// size of Tx data

		quint32 m_txDataID = 0;							// range 0..0xFFFFFFFF

		HashedVector<QString, TxRxSignalShared> m_txSignals;			// signals transmitted via port

		int m_txAnalogSignalsCount = 0;					// non-raw! tx analog signals count
		int m_txDiscreteSignalsCount = 0;				// non-raw! tx discrete signals count

		int m_txRawDataSizeW = 0;						// variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_txAnalogSignalsSizeW = 0;					//
		int m_txDiscreteSignalsSizeW = 0;				//

		bool m_txRawDataSizeWIsCalculated = false;
		bool m_txRawDataSizeWCalculationStarted = false;

		//

		int m_rxAddress = 0;							// address of port's Rx data relative from opto module appDataOffset
		int m_absRxAddress = 0;							// absoulte address of port Rx data in LM memory
		int m_rxDataSizeW = 0;							// size of Rx data

		quint32 m_rxDataID = 0;							// range 0..0xFFFFFFFF

		int m_rxRawDataSizeW = 0;						// variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_rxAnalogSignalsSizeW = 0;					//
		int m_rxDiscreteSignalsSizeW = 0;				//

		HashedVector<QString, TxRxSignalShared> m_rxSignals;			// signals received via port
	};

	typedef std::shared_ptr<OptoPort> OptoPortShared;

	typedef bool (OptoPort::*OptoPortFunc)();


	// Class represent modules with opto-ports - LM or OCM
	//
	class OptoModule : public QObject
	{
		Q_OBJECT

	public:
		OptoModule();
		~OptoModule();

		bool init(DeviceModule* module, LogicModule* lmDescription, Builder::IssueLogger* log);

		bool isLM();
		bool isOCM();

//		bool isValid() const { return m_valid; }

		QString equipmentID() const { return m_equipmentID; }
		const DeviceModule* deviceModule() const { return m_deviceModule; }

		int place() const { return m_place; }
		int optoInterfaceDataOffset() const { return m_optoInterfaceDataOffset; }
		int optoPortDataSize() const { return m_optoPortDataSize; }
		int optoPortAppDataOffset() const { return m_optoPortAppDataOffset; }
		int optoPortAppDataSize() const { return m_optoPortAppDataSize; }

		QString lmID() const { return m_lmID; }
		const DeviceModule* lmDeviceModule() const { return m_lm; }

		void getSerialPorts(QList<OptoPortShared>& serialPortsList);
		void getOptoPorts(QList<OptoPortShared>& optoPortsList);

		void getPorts(QList<OptoPortShared>& portsList);

//		QVector<OptoPort*> getPortsSorted();
//		QVector<OptoPort*> getOptoPortsSorted();

		bool calculateTxStartAddresses();

		bool addRawTxSignals(const HashedVector<QString, Signal*>& lmAssociatedSignals);
		bool addSerialRawRxSignals(const HashedVector<QString, Signal*>& lmAssociatedSignals);

		bool sortTxSignals();
		bool calculateTxSignalsAddresses();
		bool calculateTxDataIDs();
		bool calculateTxBuffersAbsAddresses();
		bool checkPortsAddressesOverlapping();

		friend class OptoModuleStorage;

	private:
		void sortPortsByEquipmentIDAscending(QVector<OptoPort*>& getPorts);

		bool forEachPort(OptoPortFunc funcPtr);

	private:
		// device properties
		//
		QString m_equipmentID;
		DeviceModule* m_deviceModule = nullptr;
		LogicModule* m_lmDescription = nullptr;

		int m_place = 0;

		int m_optoInterfaceDataOffset = 0;
		int m_optoPortDataSize = 0;
		int m_optoPortAppDataOffset = 0;
		int m_optoPortAppDataSize = 0;
		int m_optoPortCount = 0;

		//

		QString m_lmID;
		const DeviceModule* m_lm = nullptr;

		HashedVector<QString, OptoPortShared> m_ports;
		Builder::IssueLogger* m_log = nullptr;

		bool m_valid = false;

	};

	typedef std::shared_ptr<OptoModule> OptoModuleShared;


	// OptoModuleStorage is singleton!
	//
	class OptoModuleStorage : public QObject
	{
		Q_OBJECT

	public:
		struct ConnectionInfo
		{
			QString caption;
			QString port1;
			QString port2;
			std::shared_ptr<Connection> connection;
		};

	public:
		OptoModuleStorage(EquipmentSet* equipmentSet, Builder::LmDescriptionSet* lmDescriptionSet, Builder::IssueLogger* log);
		~OptoModuleStorage();

		bool build();

		static OptoModuleShared getOptoModule(const QString& optoModuleID);
		static OptoModuleShared getOptoModule(const OptoPortShared optoPort);

		static OptoPortShared getOptoPort(const QString& optoPortID);

		QString getOptoPortAssociatedLmID(OptoPortShared optoPort);

		Q_INVOKABLE Hardware::OptoPort* jsGetOptoPort(const QString& optoPortID);

		bool isCompatiblePorts(OptoPortShared optoPort1, OptoPortShared optoPort2);

		QList<OptoModuleShared> getLmAssociatedOptoModules(const QString& lmID);

		bool addRawTxSignals(const QString& lmID, const HashedVector<QString, Signal *>& lmAssociatedSignals);
		bool addSerialRawRxSignals(const QString& lmID, const HashedVector<QString, Signal*>& lmAssociatedSignals);

		bool sortTxSignals(const QString& lmID);
		bool sortSerialRxSignals(const QString& lmID);

		bool calculateTxSignalsAddresses(const QString& lmID);
		bool calculateTxDataIDs(const QString& lmID);
		bool calculateTxBuffersAbsAddresses(const QString& lmID);

		bool setPortsRxDataSizes();
		bool calculatePortsAbsoulteTxStartAddresses();
		bool calculatePortsRxStartAddresses();

		bool addConnections(const Hardware::ConnectionStorage& connectionStorage);
		std::shared_ptr<Connection> getConnection(const QString& connectionID);

		bool addRegularTxSignal(const QString& schemaID,
						 const QString& connectionID,
						 QUuid transmitterUuid,
						 const QString& lmID,
						 const Signal* appSignalID,
						 bool* signalAlreadyInList);

		bool addSerialRegularRxSignal(const QString& schemaID,
									  const QString& connectionID,
									  QUuid receiverUuid,
									  const QString& lmID,
									  const Signal* appSignal);

		void getOptoModulesSorted(QVector<OptoModuleShared>& modules);

		bool getSignalRxAddress(const QString& connectionID,
								const QString& appSignalID,
								const QString& receiverLM,
								QUuid receiverUuid,
								SignalAddress16& addr);
	private:
		QList<OptoModuleShared> modules();
		QList<OptoPortShared> ports();

		bool addModule(DeviceModule* module);

		void clear();

		bool getSignalRxAddressOpto(std::shared_ptr<Connection> connection,
									const QString& appSignalID,
									const QString& receiverLM,
									QUuid receiverUuid,
									SignalAddress16 &addr);

		bool getSignalRxAddressSerial(std::shared_ptr<Connection> connection,
										const QString& appSignalID,
										const QString& receiverLM,
										QUuid receiverUuid,
										SignalAddress16 &addr);

		bool forEachPortOfLmAssociatedOptoModules(const QString& lmID, OptoPortFunc funcPtr);

	private:
		static EquipmentSet* m_equipmentSet;
		static Builder::LmDescriptionSet* m_lmDescriptionSet;
		static Builder::IssueLogger* m_log;

		static int m_instanceCount;

		static HashedVector<QString, OptoModuleShared> m_modules;
		static HashedVector<QString, OptoPortShared> m_ports;

		static QHash<QString, OptoModuleShared> m_lmAssociatedModules;

		static QHash<QString, std::shared_ptr<Connection>> m_connections;		// connectionID -> connection
	};
}
