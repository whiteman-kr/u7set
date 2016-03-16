#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../include/OrderedHash.h"
#include "../include/ModuleConfiguration.h"
#include "../Builder/Parser.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemeItemSignal.h"
#include "../VFrame30/SchemeItemAfb.h"
#include "../VFrame30/SchemeItemConst.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/FblItem.h"
#include "Subsystem.h"
#include "Connection.h"

/*namespace Hardware {
	class ConnectionStorage;
    class Connection;
}*/

namespace Builder
{

	// Signals properties
	//
	const char* const VALUE_OFFSET = "ValueOffset";
	const char* const VALUE_BIT = "ValueBit";

	const char* const PARAM_TEST_START_COUNT = "test_start_count";

	// Constants
	//

	// FB SCALE

	//

	const int ERR_VALUE = -1;

	const int NOT_FB_OPERAND_INDEX = -1;

	const int	LM1_PLACE = 0,
				LM2_PLACE = 15,

				FIRST_MODULE_PLACE = 1,
				LAST_MODULE_PLACE = 14,

				MODULES_COUNT = 14,

				OPTO_INTERFACE_COUNT = 3;

	const int	WORD_SIZE = 16;

	const int	ANALOG_SIZE_W = 2;

	const int	SIZE_16BIT = 16;
	const int	SIZE_32BIT = 32;

	const int	FOR_USER_ONLY_PARAM_INDEX = -1;		// index of FB's parameters used by user only


	class AddrW
	{
	private:
		int m_base = 0;
		int m_offset = 0;

	public:
		AddrW() {}

		void setBase(int base) { m_base = base; }

		int base() { return m_base; }
		int offset() { return m_offset; }
		int address() { return m_base + m_offset; }

		void reset() { m_base = m_offset = 0; }

		void addWord() { m_offset++; }
		void addWord(int n) { m_offset += n; }
	};


	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
        Hardware::EquipmentSet* m_equipmentSet = nullptr;
        Hardware::DeviceObject* m_deviceRoot = nullptr;
		SignalSet* m_signals = nullptr;
		Afb::AfbElementCollection* m_afbl = nullptr;
		AppLogicData* m_appLogicData = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;

		QVector<Hardware::DeviceModule*> m_lm;

		QString msg;

	private:
		void findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool compileModulesLogics();

		bool writeBinCodeForLm(QString subsysStrID, QString lmCaption, int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode);

	public:
        ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems, Hardware::EquipmentSet* equipmentSet, SignalSet* signalSet,
								 Afb::AfbElementCollection* afblSet, AppLogicData* appLogicData,
								 BuildResultWriter* buildResultWriter, OutputLog* log, Hardware::ConnectionStorage* connections);

		bool run();

		friend class ModuleLogicCompiler;
	};


	// typedefs Logic* for types defined outside ApplicationLogicCompiler
	//

	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::SchemeItemSignal LogicSignal;
	typedef VFrame30::SchemeItemAfb LogicFb;
	typedef VFrame30::AfbPin LogicPin;
	typedef VFrame30::SchemeItemConst LogicConst;
	typedef Afb::AfbSignal LogicAfbSignal;
	typedef Afb::AfbParam LogicAfbParam;

	class AppItem;
	class ModuleLogicCompiler;


	// Functional Block Library element
	//

	class LogicAfb
	{
	private:
		std::shared_ptr<Afb::AfbElement> m_afb;

	public:
		LogicAfb(std::shared_ptr<Afb::AfbElement> afb);
		~LogicAfb();

		bool hasRam() const { return m_afb->hasRam(); }

		const Afb::AfbElement& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }
		Afb::AfbType type() const { return m_afb->type(); }
	};


	typedef QHash<int, int> FblInstanceMap;				// Key is OpCode
	typedef QHash<QString, int> NonRamFblInstanceMap;


	class AppFb;

	class AfbMap: public HashedVector<QString, LogicAfb*>
	{
	private:

		struct StrIDIndex
		{
			QString strID;		// AfbElement strID()
			int index;			// AfbElementSignal or AfbElementParam index

			operator QString() const { return QString("%1:%2").arg(strID).arg(index); }
		};

		FblInstanceMap m_fblInstance;						// Fbl opCode -> current instance
		NonRamFblInstanceMap m_nonRamFblInstance;			// Non RAM Fbl StrID -> instance

		QHash<QString, LogicAfbSignal> m_afbSignals;
		QHash<QString, LogicAfbParam*> m_afbParams;

	public:
		~AfbMap() { clear(); }

		bool addInstance(AppFb *appFb);

		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);
	};


	// Base class for AppFb & AppSignal
	// contains pointer to AppLogicItem
	//

	class AppItem : public QObject
	{
		Q_OBJECT

	protected:
		AppLogicItem m_appLogicItem;


	private:
		QHash<QString, int> m_opNameToIndexMap;

	public:
		AppItem(const AppItem& appItem);
		AppItem(const AppLogicItem& appLogicItem);
		AppItem(std::shared_ptr<Afb::AfbElement> afbElement);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.m_afbElement.strID(); }

		QString strID() const;

		bool isSignal() const { return m_appLogicItem.m_fblItem->isSignalElement(); }
		bool isFb() const { return m_appLogicItem.m_fblItem->isFblElement(); }
		bool isConst() const { return m_appLogicItem.m_fblItem->isConstElement(); }

		bool hasRam() const { return afb().hasRam(); }

		const std::list<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::list<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.m_afbElement.params(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toFblElement(); }
		const LogicConst& logicConst() const { return *m_appLogicItem.m_fblItem->toSchemeItemConst(); }
		const Afb::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }
	};


	class AppFbParamValue
	{
	private:
		E::SignalType m_type = E::SignalType::Discrete;
		E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
		bool m_instantiator = false;
		int m_dataSize = 1;
		QString m_opName;
		int m_operandIndex = NOT_FB_OPERAND_INDEX;

		quint32 m_unsignedIntValue = 0;
		qint32 m_signedIntValue = 0;
		double m_floatValue = 0;

	public:
		AppFbParamValue() {}
		AppFbParamValue(const Afb::AfbParam& afbParam);

		bool isUnsignedInt() const { return m_dataFormat == E::DataFormat::UnsignedInt; }
		bool isUnsignedInt16() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_16BIT; }
		bool isUnsignedInt32() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_32BIT; }
		bool isSignedInt32() const { return m_dataFormat == E::DataFormat::SignedInt && m_dataSize == SIZE_32BIT; }
		bool isFloat32() const { return m_dataFormat == E::DataFormat::Float && m_dataSize == SIZE_32BIT; }

		bool instantiator() const { return m_instantiator; }

		E::SignalType type() const { return m_type; }
		E::DataFormat dataFormat() const { return m_dataFormat; }
		int dataSize() const { return m_dataSize; }

		int operandIndex() const { return m_operandIndex; }
		const QString& opName() const { return m_opName; }

		quint32 unsignedIntValue() const { return m_unsignedIntValue; }
		void setUnsignedIntValue(quint32 value) { m_unsignedIntValue = value; }

		qint32 signedIntValue() const { return m_signedIntValue; }
		void setSignedIntValue(qint32 value) { m_signedIntValue = value; }

		double floatValue() const { return m_floatValue; }
		void setFloatValue(double value) { m_floatValue = value; }

		QString toString() const;
	};

	typedef HashedVector<QString, AppFbParamValue> AppFbParamValuesArray;


	// Application Functional Block
	// represent all FB items in application logic schemes
	//

	class ModuleLogicCompiler;

	class AppFb : public AppItem
	{
	private:
		quint16 m_instance = -1;
		int m_number = -1;
		QString m_instantiatorID;

		AppFbParamValuesArray m_paramValuesArray;

		ModuleLogicCompiler* m_compiler = nullptr;
		OutputLog* m_log = nullptr;

		int m_runTime = 0;

		// FB's parameters values and runtime calculations
		// implemented in file FbParamCalculation.cpp
		//

		bool calculate_LOGIC_paramValues();
		bool calculate_NOT_paramValues();
		bool calculate_SR_RS_paramValues();
		bool calculate_CTUD_paramValues();
		bool calculate_MAJ_paramValues();
		bool calculate_SRSST_paramValues();
		bool calculate_BCOD_paramValues();
		bool calculate_BDEC_paramValues();
		bool calculate_MATH_paramValues();
		bool calculate_TCT_paramValues();
		bool calculate_BCOMP_paramValues();
		bool calculate_SCALE_paramValues();
		bool calculate_LAG_paramValues();

		//

		bool checkRequiredParameters(const QStringList& requiredParams);

		bool checkUnsignedInt(const AppFbParamValue& paramValue);
		bool checkUnsignedInt16(const AppFbParamValue& paramValue);
		bool checkUnsignedInt32(const AppFbParamValue& paramValue);
		bool checkSignedInt32(const AppFbParamValue& paramValue);
		bool checkFloat32(const AppFbParamValue& paramValue);

		//

	public:
		AppFb(const AppItem &appItem);

		quint16 instance() const { return m_instance; }
		quint16 opcode() const { return afb().type().toOpCode(); }		// return FB type
		QString caption() const { return afb().caption().toUpper(); }
		QString typeCaption() const { return afb().type().text(); }
		int number() const { return m_number; }

		QString instantiatorID();

		void setInstance(quint16 instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, LogicAfbParam* afbParam) const;
		bool getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const;

		bool calculateFbParamValues(ModuleLogicCompiler *compiler);			// implemented in file FbParamCalculation.cpp

		const AppFbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

		int runTime() const { return m_runTime; }
	};


	class AppFbMap: public HashedVector<QUuid, AppFb*>
	{
	private:
		int m_fbNumber = 1;

	public:
		~AppFbMap() { clear(); }

		AppFb* insert(AppFb *appFb);
		void clear();
	};


	// Application Signal
	// represent all signal in application logic schemes, and signals, which createad in compiling time
	//

	class AppSignal //: public Signal
	{
	private:
		Signal* m_signal = nullptr;							// pointer to signal in m_signalSet

		const AppItem* m_appItem = nullptr;					// application signals pointer (for real signals)
															// application functional block pointer (for shadow signals)
		QUuid m_guid;

		bool m_isShadowSignal = false;

		bool m_computed = false;

	public:
		AppSignal(Signal* signal, const AppItem* appItem);
		AppSignal(const QUuid& guid, E::SignalType signalType, E::DataFormat dataFormat, int dataSize, const AppItem* appItem, const QString& strID);

		~AppSignal();

		const AppItem &appItem() const;

		void setComputed() { m_computed = true; }
		bool isComputed() const { return m_computed; }


		bool isShadowSignal() const { return m_isShadowSignal; }

		QString strID() const { return m_signal->strID(); }

		const Address16& ramAddr() const { return m_signal->ramAddr(); }
		const Address16& regAddr() const { return m_signal->regAddr(); }

		Address16& ramAddr() { return m_signal->ramAddr(); }
		Address16& regAddr() { return m_signal->regAddr(); }


		E::SignalType type() const { return m_signal->type(); }
		E::DataFormat dataFormat() const { return m_signal->dataFormat(); }
		int dataSize() const { return m_signal->dataSize(); }

		bool isAnalog() const { return m_signal->isAnalog(); }
		bool isDiscrete() const { return m_signal->isDiscrete(); }
		bool isRegistered() const { return m_signal->isRegistered(); }
		bool isInternal() const { return m_signal->isInternal(); }
		bool isInput() const { return m_signal->isInput(); }
		bool isOutput() const { return m_signal->isOutput(); }
		bool isCompatibleDataFormat(Afb::AfbDataFormat afbDataFormat) const { return m_signal->isCompatibleDataFormat(afbDataFormat); }

		const Signal& constSignal() { return *m_signal; }

	};


	class AppSignalMap: public QObject, public HashedVector<QUuid, AppSignal*>
	{
		Q_OBJECT

	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;

		ModuleLogicCompiler& m_compiler;

		// counters for Internal signals only
		//
		int m_registeredAnalogSignalCount = 0;
		int m_registeredDiscreteSignalCount = 0;

		int m_notRegisteredAnalogSignalCount = 0;
		int m_notRegisteredDiscreteSignalCount = 0;

		void incCounters(const AppSignal* appSignal);

		QString getShadowSignalStrID(const AppFb* appFb, const LogicPin& outputPin);

	public:
		AppSignalMap(ModuleLogicCompiler& compiler);
		~AppSignalMap();

		bool insert(const AppItem* appItem);
		bool insert(const AppFb* appFb, const LogicPin& outputPin);

		AppSignal* getByStrID(const QString& strID);

		void clear();
	};


	struct MemoryArea
	{
	public:
		struct SignalAddress16
		{
		private:
			Address16 m_address;
			QString m_signalStrID;
			int m_sizeW = 0;
			bool m_isDiscrete = false;

		public:
			SignalAddress16() {}

			SignalAddress16(const QString& strID, const Address16& address, int sizeW, bool isDiscrete) :
				m_address(address),
				m_signalStrID(strID),
				m_sizeW(sizeW),
				m_isDiscrete(isDiscrete)
			{
			}

			SignalAddress16(const SignalAddress16& sa)
			{
				m_signalStrID = sa.m_signalStrID;
				m_address = sa.m_address;
				m_sizeW = sa.m_sizeW;
				m_isDiscrete = sa.m_isDiscrete;
			}

			void setSignalStrID(const QString& strID) { m_signalStrID = strID; }
			QString signalStrID() const { return m_signalStrID; }

			void setAddress(const Address16& address) { m_address = address; }
			Address16 address() const { return m_address; }

			void setSizeW(int sizeW) { m_sizeW = sizeW; }
			int sizeW() const { return m_sizeW; }

			void setDiscrete(bool discrete) { m_isDiscrete = discrete; }
			bool isDiscrete() const { return m_isDiscrete; }
		};

	private:
		int m_startAddress = 0;
		int m_sizeW = 0;

		bool m_locked = false;

		Address16 m_nextSignalAddress;
		QVector<SignalAddress16> m_signals;

	public:
		void setStartAddress(int startAddress);

		void setSizeW(int sizeW) { assert(m_locked == false); m_sizeW = sizeW; }

		int startAddress() const { return m_startAddress; }
		int sizeW() const { return m_sizeW; }

		int* ptrStartAddress() { return &m_startAddress; }
		int* ptrSizeW() { return &m_sizeW; }

		void lock() { m_locked = true; }
		void unlock() { m_locked = false; }

		int nextAddress() const { return m_startAddress + m_sizeW; }

		MemoryArea& operator = (const MemoryArea& ma);

		Address16 appendSignal(const Signal &signal);

		bool hasSignals() const { return m_signals.size() > 0; }

		QVector<SignalAddress16>& getSignals() { return m_signals; }
	};


	class LmMemoryMap : public QObject
	{
		Q_OBJECT

	private:

		struct
		{
			MemoryArea memory;

			MemoryArea module[MODULES_COUNT];
		} m_modules;

		struct
		{
			MemoryArea memory;

			MemoryArea channel[OPTO_INTERFACE_COUNT];
			MemoryArea result;

		} m_optoInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea regDiscretSignals;
			MemoryArea nonRegDiscretSignals;

		} m_appBitAdressed;

		struct
		{
			MemoryArea memory;

		} m_tuningInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea lmDiagnostics;					// copying from this->lmDiagnostics
			MemoryArea lmInputs;						// reads from this->m_lmInOuts area
			MemoryArea lmOutputs;						// writes to this->m_lmInOuts ares
			MemoryArea module[MODULES_COUNT];			// depends from chassis	configuration
			MemoryArea regAnalogSignals;
			MemoryArea regDiscreteSignals;				// copying from this->appBitAdressed.regDiscretSignals
			MemoryArea nonRegAnalogSignals;

		} m_appWordAdressed;

		struct
		{
			MemoryArea memory;
		} m_lmDiagnostics;

		struct
		{
			MemoryArea memory;
		} m_lmInOuts;

		OutputLog* m_log = nullptr;

		void addSection(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addSignals(QStringList& memFile, MemoryArea& memArea);

	public:

		LmMemoryMap(OutputLog* log);

		bool init(	const MemoryArea& moduleData,
					const MemoryArea& optoInterfaceData,
					const MemoryArea& appLogicBitData,
					const MemoryArea& tuningData,
					const MemoryArea& appLogicWordData,
					const MemoryArea& lmDiagData,
					const MemoryArea& lmIntOutData);

		bool recalculateAddresses();

		int lmDiagnosticsAddress() const { return m_lmDiagnostics.memory.startAddress(); }
		int lmDiagnosticsSizeW() const { return m_lmDiagnostics.memory.sizeW(); }

		int lmInOutsAddress() const { return m_lmInOuts.memory.startAddress(); }
		int lmInOutsSizeW() const { return m_lmInOuts.memory.sizeW(); }

		int regDiscreteSignalsAddress() const { return m_appBitAdressed.regDiscretSignals.startAddress(); }
		int regDiscreteSignalsSizeW() const { return m_appBitAdressed.regDiscretSignals.sizeW(); }

		int bitAddressedMemoryAddress() const { return m_appBitAdressed.memory.startAddress(); }
		int wordAddressedMemoryAddress() const { return m_appWordAdressed.memory.startAddress(); }


		// rb_* - adrresses and sizes in Registration Buffer
		//
		int rb_lmDiagnosticsAddress() const { return m_appWordAdressed.lmDiagnostics.startAddress(); }
		int rb_regDiscreteSignalsAddress() const { return m_appWordAdressed.regDiscreteSignals.startAddress(); }

		int rb_lmInputsAddress() const { return m_appWordAdressed.lmInputs.startAddress(); }
		int rb_lmOutputsAddress() const { return m_appWordAdressed.lmOutputs.startAddress(); }

		//

		int getModuleDataOffset(int place);

		int getModuleRegDataOffset(int place);

		int addModule(int place, int moduleAppRegDataSize);

		void getFile(QStringList& memFile);

		Address16 addRegDiscreteSignal(const Signal& signal);
		Address16 addRegDiscreteSignalToRegBuffer(const Signal& signal);
		Address16 addNonRegDiscreteSignal(const Signal& signal);
		Address16 addRegAnalogSignal(const Signal& signal);
		Address16 addNonRegAnalogSignal(const Signal& signal);

		double bitAddressedMemoryUsed();
		double wordAddressedMemoryUsed();
	};


	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	private:

		struct PropertyNameVar
		{
			const char* name = nullptr;
			int* var = nullptr;

			PropertyNameVar(const char* n, int* v) : name(n), var(v) {}
		};

		struct Module
		{
			Hardware::DeviceModule* device = nullptr;
			int place = 0;

			// properties loaded from Hardware::DeviceModule::dynamicProperties
			//
			int txDataSize = 0;					// size of data transmitted to LM
			int rxDataSize = 0;					// size of data received from LM

			int diagDataOffset = 0;
			int diagDataSize = 0;

			int appLogicDataOffset = 0;
			int appLogicDataSize = 0;
			int appLogicDataSizeWithReserve = 0;
			int appLogicRegDataSize = 0;

			// calculated fields
			//
			int rxTxDataOffset = 0;				// offset of data received from module or transmitted to module in LM's memory
												// depends of module place in the chassis

			int moduleAppDataOffset = 0;		// offset of module application data in LM's memory
												// moduleAppDataOffset == rxTxDataOffset + appLogicDataOffset

			int appLogicRegDataOffset = 0;			// offset of module application data for processing (in registration buffer)

			bool isInputModule() const;
			bool isOutputModule() const;
			Hardware::DeviceModule::FamilyType familyType() const;
		};


		struct FbScal
		{
			QString caption;

			std::shared_ptr<Afb::AfbElement> pointer;

			int x1ParamIndex = -1;
			int x2ParamIndex = -1;
			int y1ParamIndex = -1;
			int y2ParamIndex = -1;

			int inputSignalIndex = -1;
			int outputSignalIndex = -1;
		};


		// input parameters
		//

		ApplicationLogicCompiler& m_appLogicCompiler;
        Hardware::EquipmentSet* m_equipmentSet = nullptr;
        Hardware::DeviceObject* m_deviceRoot = nullptr;
		SignalSet* m_signals = nullptr;
		Afb::AfbElementCollection* m_afbl = nullptr;
		AppLogicData* m_appLogicData = nullptr;
		AppLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;

		// compiler settings
		//

		bool m_convertUsedInOutAnalogSignalsOnly = false;

		// LM's and modules settings
		//
		int m_lmAppLogicFrameSize = 0;
		int m_lmAppLogicFrameCount = 0;

		int m_lmCycleDuration = 0;

		int m_lmClockFrequency = 96000000;
		int m_lmALPPhaseTime = 1000;
		int m_lmIDRPhaseTime = 2500;

		// LM's calculated memory offsets and sizes
		//

		LmMemoryMap m_memoryMap;

		QVector<Module> m_modules;

		//

		AddrW m_regDataAddress;

		ApplicationLogicCode m_code;

		int m_idrPhaseClockCount = 0;		// input data receive phase clock count
		int m_alpPhaseClockCount = 0;		// application logic processing clock count

		AfbMap m_afbs;

		AppSignalMap m_appSignals;
		AppFbMap m_appFbs;

		// service maps
		//
		HashedVector<QUuid, AppItem*> m_appItems;			// item GUID -> item ptr
		QHash<QUuid, AppItem*> m_pinParent;					// pin GUID -> parent item ptr
		QHash<QString, Signal*> m_signalsStrID;				// signals StrID -> Signal ptr
		QHash<QString, Signal*> m_deviceBoundSignals;		// device signal strID -> Signal ptr
		QHash<QUuid, QUuid> m_outPinSignal;					// output pin GUID -> signal GUID

		QHash<Hardware::DeviceModule::FamilyType, QString> m_moduleFamilyTypeStr;

		QString msg;

		QVector<FbScal> m_fbScal;

		const int FB_SCALE_16UI_FP_INDEX = 0;
		const int FB_SCALE_16UI_SI_INDEX = 1;
		const int FB_SCALE_FP_16UI_INDEX = 2;
		const int FB_SCALE_SI_16UI_INDEX = 3;

		QVector<AppItem*> m_scalAppItems;
		QHash<QString, AppFb*> m_inOutSignalsToScalAppFbMap;

	private:
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const QString& name, int* value);

		bool getLMIntProperty(const QString& name, int* value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		// module logic compilations steps
		//
		bool loadLMSettings();
		bool loadModulesSettings();

		bool prepareAppLogicGeneration();

		bool generateAppStartCommand();
		bool generateFbTestCode();
		bool finishTestCode();

		bool startAppLogicCode();

		bool initAfbs();

		bool copyLMDataToRegBuf();
		bool copyInModulesAppLogicDataToRegBuf();

		bool copyLmOutSignalsToModuleMemory();

		bool copyDimDataToRegBuf(const Module& module);
		bool copyAimDataToRegBuf(const Module& module);
		bool copyAifmDataToRegBuf(const Module& module);

		bool initOutModulesAppLogicDataInRegBuf();

		bool generateAppLogicCode();
		bool generateAppSignalCode(const AppItem* appItem);
		bool generateFbCode(const AppItem *appItem);

		bool writeFbInputSignals(const AppFb *appFb);
		bool startFb(const AppFb* appFb);
		bool readFbOutputSignals(const AppFb *appFb);

		bool generateReadFuncBlockToSignalCode(const AppFb& appFb, const LogicPin& outPin, const QUuid& signalGuid);

		bool generateWriteConstToSignalCode(AppSignal& appSignal, const LogicConst& constItem);
		bool generateWriteSignalToSignalCode(AppSignal &appSignal, const AppSignal& srcSignal);

		bool generateWriteConstToFbCode(const AppFb& appFb, const LogicPin& inPin, const LogicConst& constItem);
		bool generateWriteSignalToFbCode(const AppFb& appFb, const LogicPin& inPin, const AppSignal& appSignal);

		bool copyDiscreteSignalsToRegBuf();

		bool copyOutModulesAppLogicDataToModulesMemory();

        bool generateRS232ConectionCode();
        bool generateRS232ConectionCode(std::shared_ptr<Hardware::Connection> connection, Hardware::DeviceObject *port);

		bool copyDomDataToModuleMemory(const Module& module);
		bool copyAomDataToModuleMemory(const Module& module);

		bool calculateCodeRunTime();

		bool finishAppLogicCode();

		bool findFbsForAnalogInOutSignalsConversion();
		bool appendFbsForAnalogInOutSignalsConversion();
		AppItem* createFbForAnalogInputSignalConversion(const Signal &signal);
		AppItem* createFbForAnalogOutputSignalConversion(const Signal &signal);

		bool buildServiceMaps();

		bool createAppFbsMap();
		AppFb* createAppFb(const AppItem& appItem);

		bool createDeviceBoundSignalsMap();
		bool createAppSignalsMap();

		bool initAppFbParams(AppFb* appFb, bool instantiatorsOnly);

		bool getUsedAfbs();
		QString getAppLogicItemStrID(const AppLogicItem& appLogicItem) const { AppItem appItem(appLogicItem); return appItem.strID(); }

		bool copyInOutSignalsToRegistration();

		bool calculateLmMemoryMap();
		bool calculateInOutSignalsAddresses();
		bool calculateInternalSignalsAddresses();

		bool generateApplicationLogicCode();

		bool writeResult();

		void displayTimingInfo();

		void writeLMCodeTestFile();

		bool writeOcmRsSignalsXml();

		void cleanup();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		const SignalSet& signalSet() { return *m_signals; }
		Signal* getSignal(const QString& strID);

		OutputLog* log() { return m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbs.getAfbSignal(afbStrID, signalIndex); }

		bool run();

		int lmCycleDuration() const { return m_lmCycleDuration; }
	};
}


