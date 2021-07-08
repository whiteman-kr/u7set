#pragma once

#include "BuildWorkerThread.h"

// Forware delcarations
//
class QThread;
class IssueLogger;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class DeviceRoot;
	class McFirmwareOld;
}

namespace Builder
{

	// ------------------------------------------------------------------------
	//
	//		JsBusSignal
	//
	// ------------------------------------------------------------------------

	class JsBusSignal : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString SignalID READ propSignalId)
		Q_PROPERTY(QString Caption READ propCaption)

		Q_PROPERTY(E::SignalType SignalType READ propSignalType)
		Q_PROPERTY(E::AnalogAppSignalFormat AnalogFormat READ propAnalogFormat)

		Q_PROPERTY(int OffsetB READ offsetB)
		Q_PROPERTY(int OffsetW READ offsetW)
		Q_PROPERTY(int OffsetBits READ offsetBits)

		Q_PROPERTY(int SizeB READ sizeB)
		Q_PROPERTY(int SizeW READ sizeW)
		Q_PROPERTY(int SizeBits READ sizeBits)

	public:
		JsBusSignal(QObject* parent, const BusSignal* signal, int offsetW);

	public:
		QString propSignalId() const;
		QString propCaption() const;

		E::SignalType propSignalType();
		E::AnalogAppSignalFormat propAnalogFormat();

		int propInbusSizeBits() const;

		int offsetB() const;
		int offsetW() const;
		int offsetBits() const;

		int sizeB() const;
		int sizeW() const;
		int sizeBits() const;

	private:
		const BusSignal* m_signal = nullptr;
		int m_offsetW = 0;
	};

	// ------------------------------------------------------------------------
	//
	//		JsSignalSet
	//
	// ------------------------------------------------------------------------

	class JsSignalSet : public QObject
	{
		Q_OBJECT

	private:
		SignalSet* m_signalSet = nullptr;

	public:
		JsSignalSet(SignalSet* signalSet);
		Q_INVOKABLE QObject* getSignalByEquipmentID(const QString& equpmentID);
		Q_INVOKABLE QVariantList getFlatBusSignalsList(const QString& busTypeId);

	private:
		void parseFlatBusSignals(const QString& busTypeId, QVariantList& busSignals, int offset);
	};

	// ------------------------------------------------------------------------
	//
	//		ConfigurationBuilder
	//
	// ------------------------------------------------------------------------

	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
		ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, Context* context);
		virtual ~ConfigurationBuilder();

		bool build();
		bool writeDataFiles();

		Q_INVOKABLE bool jsIsInterruptRequested();

	protected:


	private:
		DbController* db();
		IssueLogger* log() const;

		bool runConfigurationScriptFile(const std::vector<Hardware::DeviceModule *> &subsystemModules, LmDescription *lmDescription);

		bool writeExtraDataFiles();
		bool writeDeviceObjectToJson(const Hardware::DeviceObject* object, QJsonObject& jParent);


	private:
		Builder::BuildResultWriter* m_buildResultWriter = nullptr;

		BuildWorkerThread* m_buildWorkerThread = nullptr;
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		std::vector<Hardware::DeviceModule*> m_fscModules;
        LmDescriptionSet *m_lmDescriptions = nullptr;
        SignalSet* m_signalSet = nullptr;
		SubsystemStorage* m_subsystems = nullptr;
		Hardware::OptoModuleStorage *m_opticModuleStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		bool m_generateExtraDebugInfo = false;
	};

}

Q_DECLARE_METATYPE(Builder::JsBusSignal*)
