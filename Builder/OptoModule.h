#pragma once

#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/Address16.h"
#include "../lib/Signal.h"

#include "RawDataDescription.h"
#include "ModulesRawData.h"
#include "../lib/Connection.h"

class LmDescription;
class OutputLog;

namespace Builder
{
	class IssueLogger;
	class LmDescriptionSet;
	class BuildResultWriter;
	class UalSignal;
	class Context;
}

namespace Hardware
{
	typedef std::shared_ptr<Connection> ConnectionShared;

	class OptoModuleStorage;

	// --------------------------------------------------------------------------------------
	//
	// TxRxSignal class declaration
	//
	// --------------------------------------------------------------------------------------

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

		bool init(const QString& nearestSignalID, const Builder::UalSignal* ualSignal);

		bool initRawSignal(const RawDataDescriptionItem& item, int offsetFromBeginningOfBuffer);

		QString appSignalID() const;
		const QStringList& appSignalIDs() const { return m_appSignalIDs; }
		QString appSignalIDsJoined() const { return appSignalIDs().join(", "); }

		bool hasSignalID(const QString& signalID) const;

		bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
		bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }
		bool isBus() const { return m_signalType == E::SignalType::Bus; }

		bool isRaw() const { return m_type == Type::Raw; }
		bool isRegular() const { return m_type == Type::Regular; }

		void setType(TxRxSignal::Type t) { m_type = t; }

		Address16 addrInBuf() const { return m_addrInBuf; }
		void setAddrInBuf(const Address16 &addr);

		E::SignalType signalType() const { return m_signalType; }
		E::DataFormat dataFormat() const { return m_dataFormat; }
		int dataSize() const { return m_dataSize; }
		E::ByteOrder byteOrder() const { return m_byteOrder; }

		QString busTypeID() const { return m_busTypeID; }

	private:
		Type m_type = Type::Regular;

		QString m_nearestSignalID;
		QStringList m_appSignalIDs;
		E::SignalType m_signalType = E::SignalType::Analog;
		E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
		E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;
		int m_dataSize = 0;					// signal size in tx(rx)Buffer in bits
		QString m_busTypeID;

		Address16 m_addrInBuf;				// relative signal address from beginning of txBuffer (rxBuffer)
	};

	typedef std::shared_ptr<TxRxSignal> TxRxSignalShared;

	// --------------------------------------------------------------------------------------
	//
	// OptoPort class declaration
	//
	// --------------------------------------------------------------------------------------

	class OptoPort : public QObject
	{
		Q_OBJECT

	public:
		const quint16 NOT_USED_PORT_ID = 0;

		static const int TX_DATA_ID_SIZE_W = sizeof(quint32) / sizeof(quint16);		// size of opto port's txDataID in words

	public:
		OptoPort(OptoModuleStorage& storage);
		virtual ~OptoPort();

		bool init(const DeviceController* controller, int portNo, LmDescription *lmDescription, Builder::IssueLogger* log);

		bool initSettings(ConnectionShared cn);

		bool appendTxSignal(const QString& nearestSignalID, const Builder::UalSignal* txSignal);
		bool initRawTxSignals();
		bool sortTxSignals();
		bool calculateTxSignalsAddresses();
		bool calculateTxDataID();

		bool appendSinglePortRxSignal(const Builder::UalSignal* rxSignal);
		bool initSinglePortRawRxSignals();
		bool sortSinglePortRxSignals();
		bool calculateSinglePortRxSignalsAddresses();
		bool calculateSinglePortRxDataID();

		bool copyOpticalPortsTxInRxSignals();

		bool writeSerialDataXml(Builder::BuildResultWriter* resultWriter) const;

		void getTxSignals(QVector<TxRxSignalShared>& txSignals) const;
		void getRxSignals(QVector<TxRxSignalShared>& rxSignals) const;

		const QVector<TxRxSignalShared>& txSignals() const { return m_txSignals; }
		const QVector<TxRxSignalShared>& rxSignals() const { return m_rxSignals; }

		int rxSignalsCount() const { return m_rxSignals.count(); }

		void getTxAnalogSignals(QVector<TxRxSignalShared>& txSignals, bool excludeRawSignals) const;
		void getTxDiscreteSignals(QVector<TxRxSignalShared>& txSignals, bool excludeRawSignals) const;

		bool isTxSignalExists(const QString& appSignalID) const;
		bool isTxSignalExists(const Builder::UalSignal* ualSignal) const;

		bool isRxSignalExists(const QString& appSignalID) const;
		bool isRxSignalExists(const Builder::UalSignal* ualSignal) const;

		bool isSerialRxSignalExists(const QString& appSignalID) const;

		bool isUsedInConnection() const;

		bool isPortToPortConnection() const { return m_connectionType ==  Connection::Type::PortToPort; }
		bool isSinglePortConnection() const { return m_connectionType ==  Connection::Type::SinglePort; }

		bool getTxSignalAbsAddress(const QString& appSignalID, SignalAddress16* addr) const;
		bool getRxSignalAbsAddress(const QString& appSignalID, SignalAddress16* addr) const;

		bool parseRawDescription();
		bool calculatePortRawDataSize();

		quint16 linkID() const { return m_linkID; }
		void setLinkID(quint16 linkID) { m_linkID = linkID; }

		Q_INVOKABLE quint16 portID() const { return linkID(); }					// rename in java!

		Q_INVOKABLE quint32 txDataID() const { return m_txDataID; }
		quint32 rxDataID() const { return m_rxDataID; }

		Q_INVOKABLE QString equipmentID() const { return m_equipmentID; }

		Q_INVOKABLE QString linkedPortID() const { return m_linkedPortID; }
		void setLinkedPortID(const QString& linkedPortID) { m_linkedPortID = linkedPortID; }

		void setDisableDataIDControl(bool disable) { m_disableDataIDControl = disable; }
		bool disableDataIDControl() const { return m_disableDataIDControl; }

		Q_INVOKABLE bool isLinked() const { return !m_linkedPortID.isEmpty(); }

		Q_INVOKABLE QString connectionID() const { return m_connectionID; }
		void setConnectionID(const QString& connectionID) { m_connectionID = connectionID; }

		Connection::Type connectionType() const { return m_connectionType; }
		void setConnectionType(Connection::Type type) { m_connectionType = type; }

		Connection::SerialMode serialMode() const { return m_serialMode; }
		Q_INVOKABLE int jsSerialMode() const { return static_cast<int>(m_serialMode); }
		void setSerialMode(Connection::SerialMode serialMode) { m_serialMode = serialMode; }

		QString serialModeStr() const;

		int txBufAddress() const { return m_txBufAddress; }
		void setTxBufAddress(int address) { m_txBufAddress = address; }

		Q_INVOKABLE int txStartAddress() const { return txBufAddress(); }		// rename in java!

		int rxBufAddress() const { return m_rxBufAddress; }
		void setRxBufAddress(int address) { m_rxBufAddress = address; }

		int txBufAbsAddress() const;
		int rxBufAbsAddress() const;

		Q_INVOKABLE bool enableDuplex() const { return m_enableDuplex; }
		void setEnableDuplex(bool enable) { m_enableDuplex = enable; }

		Q_INVOKABLE bool enableSerial() const { return m_enableSerial; }
		void setEnableSerial(bool enable) { m_enableSerial = enable; }

		const DeviceController* deviceController() const { return m_controller; }

		QString optoModuleID() const { return m_optoModuleID; }

		QString rawDataDescriptionStr() const { return m_rawDataDescriptionStr; }
		void setRawDataDescriptionStr(const QString& description) { m_rawDataDescriptionStr = description; }

		const RawDataDescription& rawDataDescription() const { return m_rawDataDescription; }

		Q_INVOKABLE int txDataSizeW() const { return m_txDataSizeW; }
		int txUsedDataSizeW() const { return m_txUsedDataSizeW; }

		int txRawDataSizeW() const { return m_txRawDataSizeW; }
		void setTxRawDataSizeW(int rawDataSizeW);

		bool hasTxRawData() const { return m_rawDataDescriptionStr.isEmpty() == false; }

		int txAnalogSignalsSizeW() const { return m_txAnalogSignalsSizeW; }
		int txBusSignalsSizeW() const { return m_txBusSignalsSizeW; }
		int txDiscreteSignalsSizeW() const { return m_txDiscreteSignalsSizeW; }

		int txSignalsCount() const { return m_txSignals.count();}

		int portNo() const { return m_portNo; }

		bool txRawDataSizeWIsCalculated() const { return m_txRawDataSizeWIsCalculated; }

		Q_INVOKABLE int rxDataSizeW() const { return m_rxDataSizeW; }
		void setRxDataSizeW(int rxDataSizeW) { m_rxDataSizeW = rxDataSizeW; }

		int rxUsedDataSizeW() const { return m_rxUsedDataSizeW; }

		int rxRawDataSizeW() const { return m_rxRawDataSizeW; }
		int rxAnalogSignalsSizeW() const { return m_rxAnalogSignalsSizeW; }
		int rxDiscreteSignalsSizeW() const { return m_rxDiscreteSignalsSizeW; }

		Q_INVOKABLE bool manualSettings() const { return m_manualSettings; }
		void setManualSettings(bool manualSettings) { m_manualSettings = manualSettings; }

		int manualTxStartAddressW() const { return m_manualTxStartAddressW; }
		void setManualTxStartAddressW(int manualTxStartAddressW) { m_manualTxStartAddressW = manualTxStartAddressW; }

		int manualTxSizeW() const { return m_manualTxSizeW; }
		void setManualTxSizeW(int manualTxSizeW) { m_manualTxSizeW = manualTxSizeW; }

		int manualRxSizeW() const { return m_manualRxSizeW; }
		void setManualRxSizeW(int manualRxSizeW) { m_manualRxSizeW = manualRxSizeW; }

		QString lmID() const { return m_lmID; }

		QString validitySignalID() const { return m_validitySignalID; }
		Address16 validitySignalAbsAddr() const { return m_validitySignalAbsAddr; }

		void writeInfo(QStringList& list) const;

	private:
		bool appendRxSignal(const Builder::UalSignal* ualSignal);

		void sortByOffsetBitNoAscending(QVector<TxRxSignalShared>& list);
		void sortByAppSignalIdAscending(QVector<TxRxSignalShared>& list);

		bool checkSignalsOffsets(const QVector<TxRxSignalShared>& signalList, int startIndex, int count) const;

		bool sortTxRxSignalList(QVector<TxRxSignalShared> &signalList);

	private:
		OptoModuleStorage& m_storage;

		QString m_equipmentID;
		int m_portNo = 0;

		Connection::Type m_connectionType = Connection::Type::PortToPort;

		bool m_enableSerial = false;

		bool m_disableDataIDControl = false;

		// if m_enableSerial == true settings
		//
		Connection::SerialMode m_serialMode = Connection::SerialMode::RS232;
		bool m_enableDuplex = false;

		bool m_manualSettings = false;
		int m_manualTxStartAddressW = 0;
		int m_manualTxSizeW = 0;
		int m_manualRxSizeW = 0;

		QString m_rawDataDescriptionStr;
		RawDataDescription m_rawDataDescription;

		QString m_lmID;
		const DeviceController* m_controller = nullptr;

		QString m_validitySignalID;
		Address16 m_validitySignalAbsAddr;

		QString m_optoModuleID;
		QString m_linkedPortID;
		QString m_connectionID;

		quint16 m_linkID = NOT_USED_PORT_ID;			// range 0..999, 0 - not used port ID, 1..999 - linked port ID

		Builder::IssueLogger* m_log = nullptr;

		HashedVector<QString, RawDataDescriptionItem> m_rawTxSignals;
		HashedVector<QString, RawDataDescriptionItem> m_rawRxSignals;

		//

		int m_txBufAddress = BAD_ADDRESS;				// address of port's Tx buffer relative to opto module appDataOffset
		quint32 m_txDataID = 0;							// range 0..0xFFFFFFFF
		int m_txDataSizeW = 0;							// size of port's Tx data
		int m_txUsedDataSizeW = 0;						// for ports with manual settings may be m_txUsedDataSizeW < m_txDataSizeW
		int m_txRawDataSizeW = 0;
		int m_txAnalogSignalsSizeW = 0;
		int m_txBusSignalsSizeW = 0;
		int m_txDiscreteSignalsSizeW = 0;

		bool m_txRawDataSizeWIsCalculated = false;
		bool m_txRawDataSizeWCalculationStarted = false;

		QVector<TxRxSignalShared> m_txSignals;			// signals transmitted via port
		QHash<QString, TxRxSignalShared> m_txSignalIDs;

		//

		int m_rxBufAddress = BAD_ADDRESS;				// address of port's Rx buffer relative to opto module appDataOffset
		quint32 m_rxDataID = 0;							// range 0..0xFFFFFFFF
		int m_rxDataSizeW = 0;							// size of Rx data
		int m_rxUsedDataSizeW = 0;						// for ports with manual settings may be m_rxUsedDataSizeW < m_rxDataSizeW
		int m_rxRawDataSizeW = 0;						// variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_rxAnalogSignalsSizeW = 0;					//
		int m_rxBusSignalsSizeW = 0;					//
		int m_rxDiscreteSignalsSizeW = 0;				//

		QVector<TxRxSignalShared> m_rxSignals;			// signals received via port
		QHash<QString, TxRxSignalShared> m_rxSignalIDs;
	};

	typedef std::shared_ptr<OptoPort> OptoPortShared;
	typedef bool (OptoPort::*OptoPortFunc)();

	// --------------------------------------------------------------------------------------
	//
	// OptoModule class declaration
	//
	// Class is represents modules with opto-ports - LM or OCM
	//
	// --------------------------------------------------------------------------------------

	class OptoModule : public QObject
	{
		Q_OBJECT

	public:
		OptoModule(OptoModuleStorage& storage);
		virtual ~OptoModule();

		bool init(DeviceModule* module, LmDescription* lmDescription, Builder::IssueLogger* log);

		bool isLmOrBvb() const;
		bool isOcm() const;
		bool isBvb() const;

		QString equipmentID() const { return m_equipmentID; }
		const DeviceModule* deviceModule() const { return m_deviceModule; }

		int place() const { return m_place; }
		int optoInterfaceDataOffset() const { return m_optoInterfaceDataOffset; }
		int optoPortDataSize() const { return m_optoPortDataSize; }
		int optoPortAppDataOffset() const { return m_optoPortAppDataOffset; }
		int optoPortAppDataSize() const { return m_optoPortAppDataSize; }

		QString lmID() const { return m_lmID; }
		const DeviceModule* lmDeviceModule() const { return m_lm; }

		void getSerialPorts(QList<OptoPortShared>& serialPortsList) const;
		void getOptoPorts(QList<OptoPortShared>& optoPortsList) const;

		const HashedVector<QString, OptoPortShared>& ports() const { return m_ports; }

		bool calculateTxBufAddresses();
		bool checkPortsAddressesOverlapping() const;
		bool calculateRxBufAddresses();

		bool forEachPort(OptoPortFunc funcPtr);

		bool isSerialRxSignalExists(const QString& appSignalID) const;

		bool calculateTxSignalsAddresses();
		bool copyOpticalPortsTxInRxSignals();

	private:
		void sortPortsByEquipmentIDAscending(QVector<OptoPort*>& getPorts);

	private:
		OptoModuleStorage& m_storage;

		// device properties
		//
		QString m_equipmentID;
		DeviceModule* m_deviceModule = nullptr;
		LmDescription* m_lmDescription = nullptr;

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
	typedef bool (OptoModule::*OptoModuleFunc)();

	// --------------------------------------------------------------------------------------
	//
	// OptoModuleStorage class declaration
	//
	// --------------------------------------------------------------------------------------

	//class Context;

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
		OptoModuleStorage(Builder::Context* context);

		~OptoModuleStorage();

		bool init();

		bool appendOptoModules();
		bool appendAndCheckConnections();

		bool sortTxSignals(const QString& lmID);
		bool sortSerialRxSignals(const QString& lmID);

		bool initRawTxSignals(const QString& lmID);
		bool calculateTxSignalsAddresses(const QString& lmID);
		bool calculateTxDataIDs(const QString& lmID);
		bool calculateTxBufAddresses(const QString& lmID);

		bool initSerialRawRxSignals(const QString& lmID);
		bool calculateSerialRxSignalsAddresses(const QString& lmID);
		bool calculateSerialRxDataIDs(const QString& lmID);
		bool calculateRxBufAddresses(const QString& lmID);

		bool copyOpticalPortsTxInRxSignals(const QString& lmID);

		bool writeSerialDataXml(Builder::BuildResultWriter* resultWriter) const;

		bool appendTxSignal(const QString& schemaID,
						 const QString& connectionID,
						 QUuid transmitterUuid,
						 const QString& lmID,
						 const QString& nearestSignalID,
						 const Builder::UalSignal* ualSignal,
						 bool* signalAlreadyInList);

		bool appendSinglePortRxSignal(const QString& schemaID,
									  const QString& connectionID,
									  QUuid receiverUuid,
									  const QString& lmID,
									  const Builder::UalSignal* ualSignal);

		bool getRxSignalAbsAddress(const QString& schemaID,
								   const QString& connectionID,
								   const QString& appSignalID,
								   const QString& receiverLM,
								   QUuid receiverUuid,
								   SignalAddress16* addr);

		std::shared_ptr<Connection> getConnection(const QString& connectionID) const;

		OptoModuleShared getOptoModule(const QString& optoModuleID) const;
		OptoModuleShared getOptoModule(const OptoPortShared optoPort) const;
		QList<OptoModuleShared> getLmAssociatedOptoModules(const QString& lmID) const;
		void getOptoModulesSorted(QVector<OptoModuleShared>& modules) const;

		OptoPortShared getOptoPort(const QString& optoPortID) const;
		bool getLmAssociatedOptoPorts(const QString& lmID, QList<OptoPortShared>& associatedPorts) const;

		OptoPortShared getLmAssociatedOptoPort(const QString& lmID, const QString& connectionID) const;

		QString getOptoPortAssociatedLmID(OptoPortShared optoPort) const;

		bool getOptoPortValidityAbsAddr(const QString& lmID,
											   const QString& connectionID,
											   const QString& schemaID,
											   QUuid receiverUuid,
											   Address16& validityAddr) const;

		Q_INVOKABLE Hardware::OptoPort* jsGetOptoPort(const QString& optoPortID);

		bool isSerialRxSignalExists(const QString& lmID, const QString& appSignalID) const;

		bool isConnectionAccessible(const QString& lmEquipmentID, const QString& connectionID) const;

		int getAllNativeRawDataSize(const Hardware::DeviceModule* lm, Builder::IssueLogger* log);
		int getModuleRawDataSize(const Hardware::DeviceModule* lm, int modulePlace, bool* moduleIsFound, Builder::IssueLogger* log);
		int getModuleRawDataSize(const Hardware::DeviceModule* module, Builder::IssueLogger* log);

		ModuleRawDataDescription* getModuleRawDataDescription(const Hardware::DeviceModule* module) const;

	private:
		bool processConnection(ConnectionShared connection);
		void prepareLmsAccessibleConnections();

		bool addModule(DeviceModule* module);

		bool forEachPortOfLmAssociatedOptoModules(const QString& lmID, OptoPortFunc funcPtr);
		bool forEachOfLmAssociatedOptoModules(const QString& lmID, OptoModuleFunc funcPtr);

		void clear();

	private:
		std::shared_ptr<EquipmentSet> m_equipmentSet;
		std::shared_ptr<Builder::LmDescriptionSet> m_lmDescriptionSet;
		std::shared_ptr<ConnectionStorage> m_connectionStorage;
		Builder::IssueLogger* m_log = nullptr;

		HashedVector<QString, OptoModuleShared> m_modules;
		HashedVector<QString, OptoPortShared> m_ports;

		QHash<QString, OptoModuleShared> m_lmAssociatedModules;

		QHash<QString, ConnectionShared> m_connections;		// connectionID -> connection

		QHash<QString, QHash<QString, bool>> m_lmsAccessibleConnections;

		QHash<QString, ModuleRawDataDescription*> m_modulesRawDataDescription;
	};
}
