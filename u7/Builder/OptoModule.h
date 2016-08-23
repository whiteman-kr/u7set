#pragma once

#include <QObject>

#include "../lib/OrderedHash.h"
#include "../lib/DeviceObject.h"
#include "../lib/Address16.h"
#include "../lib/Signal.h"
#include "IssueLogger.h"


namespace Hardware
{
	class DeviceModule;
	class Connection;
	class ConnectionStorage;

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

		struct TxSignal
		{
			QString appSignalID;
			Address16 address;        // offset from beginning of txBuffer
			int sizeBit = 0;
		};

	private:
		QString m_equipmentID;
		DeviceController* m_deviceController = nullptr;

		QString m_optoModuleID;

		QString m_linkedPortID;

		QString m_connectionID;

		QHash<QString, Address16> m_txSignalsIDs;		// appSignalID => txAddress

		QVector<TxSignal> m_txAnalogSignals;
		QVector<TxSignal> m_txDiscreteSignals;

		int m_txAnalogSignalsSizeW = 0;     // variables is calculateed inside OptoPort::calculateTxSignalsAddresses()
		int m_txDiscreteSignalsSizeW = 0;   //
		int m_txDataSizeW = 0;              //
		quint32 m_txDataID = 0;             // range 0..0xFFFFFFFF

		int m_port = 0;

		Mode m_mode = Mode::Optical;
		SerialMode m_serialMode = SerialMode::RS232;

		int m_rxStartAddress = 0;
		int m_rxDataSizeW = 0;

		int m_txStartAddress = 0;

		quint16 m_portID = 0;           // range 0..999, 0 - not linked port ID, 1..999 - linked port ID

		// serial RS323/485 mode properies only
		//
		bool m_enable = true;
		bool m_enableDuplex = false;    // serial mode and OCMN only

		bool m_manualSettings = false;
		int m_manualTxSizeW = 0;
		int m_manualRxSizeW = 0;

		void sortTxSignals();
		void sortTxSignals(QVector<TxSignal> &array);

	public:
		OptoPort(const QString& optoModuleID, DeviceController* optoPortController, int port);

		Q_INVOKABLE quint16 portID() const { return m_portID; }
		void setPortID(quint16 portID) { m_portID = portID; }

		Q_INVOKABLE quint32 txDataID() const { return m_txDataID; }

		Q_INVOKABLE QString equipmentID() const { return m_equipmentID; }

		QString linkedPortID() const { return m_linkedPortID; }
		void setLinkedPortID(const QString& linkedPortID) { m_linkedPortID = linkedPortID; }

		bool isLinked() const { return !m_linkedPortID.isEmpty(); }

		Q_INVOKABLE QString connectionID() const { return m_connectionID; }
		void setConnectionID(const QString& connectionID) { m_connectionID = connectionID; }

		int port() const { return m_port; }
		void setPort(int port) { m_port = port; }

		Q_INVOKABLE Mode mode() const { return m_mode; }
		void setMode(Mode mode) { m_mode = mode; }

		Q_INVOKABLE SerialMode serialMode() const { return m_serialMode; }
		void setSerialMode(SerialMode serialMode) { m_serialMode = serialMode; }

		int rxStartAddress() const { return m_rxStartAddress; }
		void setRxStartAddress(int address) { m_rxStartAddress = address; }

		Q_INVOKABLE int txStartAddress() const { return m_txStartAddress; }
		void setTxStartAddress(int address) { m_txStartAddress = address; }

		Q_INVOKABLE bool enable() const { return m_enable; }
		void setEnable(bool enable) { m_enable = enable; }

		Q_INVOKABLE bool enableDuplex() const { return m_enableDuplex; }
		void setEnableDuplex(bool enable) { m_enableDuplex = enable; }

		const DeviceController* deviceController() const { return m_deviceController; }

		QString optoModuleID() const { return m_optoModuleID; }

		QVector<TxSignal> getTxSignals();

		void addTxSignal(Signal* txSignal);
		bool calculateTxSignalsAddresses(OutputLog* log);

		QVector<TxSignal> txAnalogSignals() const { return m_txAnalogSignals; }
		QVector<TxSignal> txDiscreteSignals() const { return m_txDiscreteSignals; }

		int txAnalogSignalsSizeW() const { return m_txAnalogSignalsSizeW; }
		int txAnalogSignalsCount() const { return m_txAnalogSignals.count(); }

		int txDiscreteSignalsSizeW() const { return m_txDiscreteSignalsSizeW; }
		int txDiscreteSignalsCount() const { return m_txDiscreteSignals.count(); }

		Q_INVOKABLE int txDataSizeW() const { return m_txDataSizeW; }

		Q_INVOKABLE int rxDataSizeW() const { return m_rxDataSizeW; }
		void setRxDataSizeW(int rxDataSizeW) { m_rxDataSizeW = rxDataSizeW; }

		Q_INVOKABLE bool manualSettings() const { return m_manualSettings; }
		void setManualSettings(bool manualSettings) { m_manualSettings = manualSettings; }

		int manualTxSizeW() const { return m_manualTxSizeW; }
		void setManualTxSizeW(int manualTxSizeW) { m_manualTxSizeW = manualTxSizeW; }

		int manualRxSizeW() const { return m_manualRxSizeW; }
		void setManualRxSizeW(int manualRxSizeW) { m_manualRxSizeW = manualRxSizeW; }

		bool isTxSignalIDExists(const QString& appSignalID);

		bool isConnected() const;

		Address16 getTxSignalAddress(const QString& appSignalID) const;
	};


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
		OptoModule(DeviceModule* module, Builder::IssueLogger* log);
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

		int allOptoPortsTxDataSizeW();

		friend class OptoModuleStorage;
	};


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
		Builder::IssueLogger* m_log = nullptr;

		HashedVector<QString, OptoModule*> m_modules;
		HashedVector<QString, OptoPort*> m_ports;

		QHash<QString, OptoModule*> m_lmAssociatedModules;

		QHash<QString, std::shared_ptr<Connection>> m_connections;		// connectionID -> connection

		QList<OptoModule*> modules();
		QList<OptoPort*> ports();

		bool addModule(DeviceModule* module);

		void clear();

	public:
		OptoModuleStorage(EquipmentSet* equipmentSet, Builder::IssueLogger* log);
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
		bool calculatePortsTxStartAddresses();
		bool calculatePortsRxStartAddresses();

		bool addConnections(const Hardware::ConnectionStorage& connectionStorage);
		std::shared_ptr<Connection> getConnection(const QString& connectionID);

		bool addTxSignal(const QString& connectionID,
						 const QString& lmID,
						 Signal* appSignal,
						 bool* signalAllreadyInList);

		QVector<OptoModule*> getOptoModulesSorted();

		bool getSignalRxAddress(QString connectionID, QString appSignalID, QString receiverLM, QUuid receiverUuid, Address16& addr);
	};
}
