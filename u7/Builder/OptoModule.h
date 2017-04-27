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
			RegularTx,				// regular transmitted signal
			RegularRx,				// regular received signal
			RawTx,					// raw transmitted signal
			RawRx					// raw received signal
		};

		TxRxSignal();

		bool init(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type txRxType);

		QString appSignalID() const { return m_appSignalID; }

		bool isRaw() const { return m_type == Type::RawRx || m_type == Type::RawTx; }
		bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
		bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

		Address16 addrInBuf() const { return m_addrInBuf; }
		void setAddrInBuf(Address16& addr);

		int sizeB() const { return m_sizeB; }

	private:
		Type m_type = Type::RegularTx;

		QString m_appSignalID;
		E::SignalType m_signalType = E::SignalType::Analog;
		int m_sizeB = 0;					// signal size in tx(rx)Buffer in bits

		Address16 m_addrInBuf;				// signal address from beginning of txBuffer (rxBuffer)
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

		const int TX_DATA_ID_SIZE_W = sizeof(quint32) / sizeof(quint16);		// size of opto port's txDataID in words

	public:
		OptoPort(const DeviceController* controller, int portNo);

		bool appendRegularTxSignal(const Signal* s, Builder::IssueLogger *log);

		Q_INVOKABLE quint16 portID() const { return m_portID; }
		void setPortID(quint16 portID) { m_portID = portID; }

		Q_INVOKABLE quint32 txDataID() const { return m_txDataID; }

		Q_INVOKABLE QString equipmentID() const { return m_equipmentID; }

		Q_INVOKABLE QString linkedPortID() const { return m_linkedPortID; }
		void setLinkedPortID(const QString& linkedPortID) { m_linkedPortID = linkedPortID; }

		Q_INVOKABLE bool isLinked() const { return !m_linkedPortID.isEmpty(); }

		Q_INVOKABLE QString connectionID() const { return m_connectionID; }
		void setConnectionID(const QString& connectionID) { m_connectionID = connectionID; }

		//int port() const { return m_port; }					// is need ??
		//void setPort(int port) { m_port = port; }			//

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

		void getTxSignals(QVector<TxRxSignalShared> &txSignals) const;

		bool addTxSignal(const Signal* txSignal);

		QVector<TxRxSignalShared> txAnalogSignals() const;
		QVector<TxRxSignalShared> txDiscreteSignals() const;

		bool calculateTxSignalsAddresses(Builder::IssueLogger* log);

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

		bool isTxSignalExists(const QString& appSignalID);

		bool isUsedInConnection() const;

		Address16 getTxSignalAddrInBuf(const QString& appSignalID) const;
		Address16 getTxSignalAbsAddr(const QString& appSignalID) const;

		bool parseRawDescription(Builder::IssueLogger* log);
		bool calculatePortRawDataSize(OptoModuleStorage* optoStorage, Builder::IssueLogger* log);

		bool getSignalRxAddressSerial(std::shared_ptr<Connection> connection,
										const QString& appSignalID,
										QUuid receiverUuid,
										SignalAddress16 &addr,
										Builder::IssueLogger* log);
	private:
		bool appendTxRxSignal(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type txRxType, Builder::IssueLogger* log);
		bool appendRawTxRxSignals(Builder::IssueLogger* log);

		void sortByOffsetBitNoAscending(QVector<TxRxSignal>& signalList);
		bool checkSignalsOffsets(const QVector<TxRxSignal>& signalList, bool overlapIsError, Builder::IssueLogger* log);

		void sortTxRxSignals();
		void sortTxRxSignals(HashedVector<QString, TxRxSignalShared>& array, int startIndex, int count);

		DeviceModule* getLM();

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

		//

		const DeviceController* m_controller = nullptr;

		QString m_optoModuleID;
		QString m_linkedPortID;
		QString m_connectionID;

		//

		quint16 m_portID = NOT_USED_PORT_ID;			// range 0..999, 0 - not used port ID, 1..999 - linked port ID

/*		QVector<TxRxSignal> m_txRawSignals;				// sorted by ascending of Offset:BitNo
		QVector<TxRxSignal> m_txAnalogSignals;			// sorted by ascending of AppSignalID
		QVector<TxRxSignal> m_txDiscreteSignals;		// sorted by ascending of AppSignalID*/

//		QHash<QString, Address16> m_txSignalsIDs;		// appSignalID => txAddress


		int m_txStartAddress = 0;						// address of port's Tx data relative from opto module appDataOffset
		int m_absTxStartAddress = 0;					// absoulte address of port Tx data in LM memory
		int m_txDataSizeW = 0;							// size of Tx data

		quint32 m_txDataID = 0;							// range 0..0xFFFFFFFF

		HashedVector<QString, TxRxSignalShared> m_txRxSignals;
		int m_txAnalogSignalsCount = 0;					// non-raw! tx analog signals count
		int m_txDiscreteSignalsCount = 0;				// non-raw! tx discrete signals count

		int m_txRawDataSizeW = 0;						// variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_txAnalogSignalsSizeW = 0;					//
		int m_txDiscreteSignalsSizeW = 0;				//

		bool m_txRawDataSizeWIsCalculated = false;
		bool m_txRawDataSizeWCalculationStarted = false;

//		QVector<TxRxSignal> m_rxRawSignals;				// sorted by ascending of Offset:BitNo

		int m_rxAddress = 0;							// address of port's Rx data relative from opto module appDataOffset
		int m_absRxAddress = 0;							// absoulte address of port Rx data in LM memory
		int m_rxDataSizeW = 0;							// size of Rx data

		int m_rxRawDataSizeW = 0;						// variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_rxAnalogSignalsSizeW = 0;					//
		int m_rxDiscreteSignalsSizeW = 0;				//


		HashedVector<QString, TxRxSignalShared> m_rxSignals;
	};

	typedef std::shared_ptr<OptoPort> OptoPortShared;


	// Class represent modules with opto-ports - LM or OCM
	//
	class OptoModule : public QObject
	{
		Q_OBJECT

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
		DeviceModule* m_lmDeviceModule = nullptr;

		HashedVector<QString, OptoPort*> m_ports;
		OutputLog* m_log = nullptr;

		bool m_valid = false;

		void sortPortsByEquipmentIDAscending(QVector<OptoPort*>& ports);

	public:
		OptoModule(DeviceModule* module, LogicModule* lmDescription, Builder::IssueLogger* log);
		~OptoModule();

		bool isValid() const { return m_valid; }

		bool isLM();
		bool isOCM();

		QString equipmentID() const { return m_equipmentID; }
		const DeviceModule* deviceModule() const { return m_deviceModule; }

		int place() const { return m_place; }
		int optoInterfaceDataOffset() const { return m_optoInterfaceDataOffset; }
		int optoPortDataSize() const { return m_optoPortDataSize; }
		int optoPortAppDataOffset() const { return m_optoPortAppDataOffset; }
		int optoPortAppDataSize() const { return m_optoPortAppDataSize; }

		QString lmID() const { return m_lmID; }
		const DeviceModule* lmDeviceModule() const { return m_lmDeviceModule; }

		QList<OptoPort*> getSerialPorts();
		QList<OptoPort*> getOptoPorts();

		QList<OptoPort*> ports();

		QVector<OptoPort*> getPortsSorted();
		QVector<OptoPort*> getOptoPortsSorted();

		bool calculateTxStartAddresses();

		friend class OptoModuleStorage;
	};

	typedef std::shared_ptr<OptoModule> OptoModuleShared;


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

	private:
		EquipmentSet* m_equipmentSet = nullptr;
		Builder::LmDescriptionSet* m_lmDescriptionSet = nullptr;
		Builder::IssueLogger* m_log = nullptr;

		HashedVector<QString, OptoModule*> m_modules;
		HashedVector<QString, OptoPort*> m_ports;

		QHash<QString, OptoModule*> m_lmAssociatedModules;

		QHash<QString, std::shared_ptr<Connection>> m_connections;		// connectionID -> connection

		QList<OptoModule*> modules();
		QList<OptoPort*> ports();

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

	public:
		OptoModuleStorage(EquipmentSet* equipmentSet, Builder::LmDescriptionSet* lmDescriptionSet, Builder::IssueLogger* log);
		~OptoModuleStorage();

		bool build();

		OptoModule* getOptoModule(const QString& optoModuleID);
		OptoModule* getOptoModule(const OptoPort* optoPort);

		OptoPort* getOptoPort(const QString& optoPortID);

		QString getOptoPortAssociatedLmID(const OptoPort* optoPort);

		Q_INVOKABLE Hardware::OptoPort* jsGetOptoPort(const QString& optoPortID);

		bool isCompatiblePorts(const OptoPort* optoPort1, const OptoPort* optoPort2);

		QList<OptoModule*> getLmAssociatedOptoModules(const QString& lmStrID);

		bool setPortsRxDataSizes();
		bool calculatePortsAbsoulteTxStartAddresses();
		bool checkPortsAddressesOverlapping(OptoModule* module);
		bool calculatePortsRxStartAddresses();

		bool addConnections(const Hardware::ConnectionStorage& connectionStorage);
		std::shared_ptr<Connection> getConnection(const QString& connectionID);

		bool addTxSignal(const QString& schemaID,
						 const QString& connectionID,
						 QUuid transmitterUuid,
						 const QString& lmID,
						 Signal* appSignal,
						 bool* signalAlreadyInList);

		QVector<OptoModule*> getOptoModulesSorted();

		bool getSignalRxAddress(const QString& connectionID,
								const QString& appSignalID,
								const QString& receiverLM,
								QUuid receiverUuid,
								SignalAddress16& addr);
	};
}
