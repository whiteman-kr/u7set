#pragma once

#include <QtCore>

#include "../lib/Signal.h"
#include "../lib/WUtils.h"

#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"

#include "Parser.h"
#include "ApplicationLogicCode.h"

namespace Builder
{
	// typedefs Logic* for types defined outside ModuleLogicCompiler
	//
	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::SchemaItemSignal LogicSignal;
	typedef VFrame30::SchemaItemAfb LogicFb;
	typedef VFrame30::AfbPin LogicPin;
	typedef VFrame30::SchemaItemConst LogicConst;
	typedef VFrame30::SchemaItemTransmitter LogicTransmitter;
	typedef VFrame30::SchemaItemReceiver LogicReceiver;
	typedef Afb::AfbSignal LogicAfbSignal;
	typedef Afb::AfbParam LogicAfbParam;

	class AppItem;
	class ModuleLogicCompiler;

	class LogicAfb
	{
		// Functional Block Library element
		//
	public:
		LogicAfb(std::shared_ptr<Afb::AfbElement> afb);
		virtual ~LogicAfb();

		bool hasRam() const { return m_afb->hasRam(); }
		int opCode() const	{ return m_afb->opCode(); }

		const Afb::AfbElement& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }

	private:
		std::shared_ptr<Afb::AfbElement> m_afb;
	};

	typedef QHash<int, int> FblInstanceMap;				// Key is OpCode
	typedef QHash<QString, int> NonRamFblInstanceMap;

	class AppFb;

	class AfbMap : public HashedVector<QString, LogicAfb*>
	{
	public:
		virtual ~AfbMap();

		bool addInstance(AppFb *appFb);
		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);

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
	};

	class AppItem : public QObject
	{
		// Base class for AppFb & AppSignal
		// contains pointer to AppLogicItem
		//
		Q_OBJECT

	public:
		enum Type
		{
			Unknown,
			Signal,
			Fb,
			Const,
			Transmitter,
			Receiver,
			Terminator
		};

		AppItem();
		AppItem(const AppItem& appItem);
		AppItem(const AppLogicItem& appLogicItem);
		AppItem(std::shared_ptr<Afb::AfbElement> afbElement, QString &errorMsg);

		bool init(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.m_afbElement.strID(); }
		QString caption() const { return m_appLogicItem.m_afbElement.caption(); }

		QString strID() const;

		bool isSignal() const { return m_appLogicItem.m_fblItem->isSignalElement(); }
		bool isFb() const { return m_appLogicItem.m_fblItem->isAfbElement(); }
		bool isConst() const { return m_appLogicItem.m_fblItem->isConstElement(); }
		bool isTransmitter() const { return m_appLogicItem.m_fblItem->isTransmitterElement(); }
		bool isReceiver() const { return m_appLogicItem.m_fblItem->isReceiverElement(); }
		bool isTerminator() const { return m_appLogicItem.m_fblItem->isTerminatorElement(); }

		Type type() const;

		bool hasRam() const { return afb().hasRam(); }

		const std::vector<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::vector<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.m_afbElement.params(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toAfbElement(); }
		const LogicConst& logicConst() const { return *m_appLogicItem.m_fblItem->toSchemaItemConst(); }
		const LogicTransmitter& logicTransmitter() const { return *m_appLogicItem.m_fblItem->toTransmitterElement(); }
		const LogicReceiver& logicReceiver() const { return *m_appLogicItem.m_fblItem->toReceiverElement(); }
		const Afb::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }

		std::shared_ptr<VFrame30::FblItemRect> itemRect() const { return m_appLogicItem.m_fblItem; }

		QString schemaID() const { return m_appLogicItem.m_schema->schemaId(); }

		QString label() const { return m_appLogicItem.m_fblItem->label(); }

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }

	protected:
		AppLogicItem m_appLogicItem;

	private:
		QHash<QString, int> m_opNameToIndexMap;
	};

	class AppFbParamValue
	{
	public:
		static const int NOT_FB_OPERAND_INDEX = -1;

	public:
		AppFbParamValue();
		AppFbParamValue(const Afb::AfbParam& afbParam);

		bool isUnsignedInt() const { return m_dataFormat == E::DataFormat::UnsignedInt; }
		bool isUnsignedInt16() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_16BIT; }
		bool isUnsignedInt32() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_32BIT; }
		bool isSignedInt32() const { return m_dataFormat == E::DataFormat::SignedInt && m_dataSize == SIZE_32BIT; }
		bool isFloat32() const { return m_dataFormat == E::DataFormat::Float && m_dataSize == SIZE_32BIT; }
		bool isVisible() const { return m_visible; }
		bool isNoFbOperand() const { return m_operandIndex == NOT_FB_OPERAND_INDEX; }

		bool instantiator() const { return m_instantiator; }

		E::SignalType type() const { return m_type; }
		E::DataFormat dataFormat() const { return m_dataFormat; }
		int dataSize() const { return m_dataSize; }

		int operandIndex() const { return m_operandIndex; }
		QString opName() const { return m_opName; }
		QString caption() const { return m_caption; }

		quint32 unsignedIntValue() const;
		void setUnsignedIntValue(quint32 value);

		qint32 signedIntValue() const;
		void setSignedIntValue(qint32 value);

		double floatValue() const;
		void setFloatValue(double value);

		QString toString() const;

	private:
		E::SignalType m_type = E::SignalType::Discrete;
		E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
		bool m_instantiator = false;
		int m_dataSize = 1;
		QString m_opName;
		QString m_caption;
		int m_operandIndex = NOT_FB_OPERAND_INDEX;
		bool m_visible = false;

		quint32 m_unsignedIntValue = 0;
		qint32 m_signedIntValue = 0;
		double m_floatValue = 0;
	};

	typedef HashedVector<QString, AppFbParamValue> AppFbParamValuesArray;

	class ModuleLogicCompiler;

	class AppFb : public AppItem
	{
		// Application Functional Block
		// represent all FB items in application logic schemas
		//
	public:
		static const int FOR_USER_ONLY_PARAM_INDEX = -1;				// index of FB's parameters used by user only

	public:
		AppFb(const AppItem &appItem);

		quint16 instance() const { return m_instance; }
		quint16 opcode() const { return afb().opCode(); }		// return FB type
		QString caption() const { return afb().caption(); }
		QString typeCaption() const { return afb().componentCaption(); }
		int number() const { return m_number; }

		bool isConstComaparator() const;
		bool isDynamicComaparator() const;
		bool isComparator() const;

		QString instantiatorID();

		void setInstance(quint16 instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, LogicAfbParam* afbParam) const;
		bool getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const;

		bool calculateFbParamValues(ModuleLogicCompiler *compiler);			// implemented in file FbParamCalculation.cpp

		const AppFbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

		int runTime() const { return m_runTime; }

	private:
		// FB's parameters values and runtime calculations
		// implemented in file FbParamCalculation.cpp
		//
		bool calculate_LOGIC_paramValues();
		bool calculate_NOT_paramValues();
		bool calculate_FLIP_FLOP_paramValues();
		bool calculate_CTUD_paramValues();
		bool calculate_MAJ_paramValues();
		bool calculate_SRSST_paramValues();
		bool calculate_BCOD_paramValues();
		bool calculate_BDEC_paramValues();
		bool calculate_MATH_paramValues();
		bool calculate_TCT_paramValues();
		bool calculate_BCOMP_paramValues();
		bool calculate_SCALE_paramValues();
		bool calculate_SCALE_P_paramValues();
		bool calculate_DAMPER_paramValues();
		bool calculate_MEM_paramValues();
		bool calculate_FUNC_paramValues();
		bool calculate_INT_paramValues();
		bool calculate_DPCOMP_paramValues();
		bool calculate_MUX_paramValues();
		bool calculate_LATCH_paramValues();
		bool calculate_LIM_paramValues();
		bool calculate_DEAD_ZONE_paramValues();
		bool calculate_POL_paramValues();
		bool calculate_DER_paramValues();
		bool calculate_MISMATCH_paramValues();
		bool calculate_TCONV_paramValues();

		//

		bool checkRequiredParameters(const QStringList& requiredParams);
		bool checkRequiredParameters(const QStringList& requiredParams, bool displayError);

		bool checkUnsignedInt(const AppFbParamValue& paramValue);
		bool checkUnsignedInt16(const AppFbParamValue& paramValue);
		bool checkUnsignedInt32(const AppFbParamValue& paramValue);
		bool checkSignedInt32(const AppFbParamValue& paramValue);
		bool checkFloat32(const AppFbParamValue& paramValue);

	private:
		quint16 m_instance = -1;
		int m_number = -1;
		QString m_instantiatorID;

		AppFbParamValuesArray m_paramValuesArray;

		ModuleLogicCompiler* m_compiler = nullptr;
		IssueLogger* m_log = nullptr;

		int m_runTime = 0;

		const quint16 CONST_COMPARATOR_OPCODE = 10;
		const quint16 DYNAMIC_COMPARATOR_OPCODE = 20;
	};


	class AppFbMap: public HashedVector<QUuid, AppFb*>
	{
	private:
		int m_fbNumber = 1;

	public:
		virtual ~AppFbMap();

		AppFb* insert(AppFb *appFb);
		void clear();
	};

	class AppSignal
	{
		// Application Signal
		// represent all signal in application logic schemas, and signals, which createad in compiling time
		//
	public:
		AppSignal(Signal* signal, const AppItem* appItem);
		AppSignal(const QUuid& guid, E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, int dataSize, const AppItem* appItem, const QString& appSignalID);

		~AppSignal();

		const AppItem &appItem() const;

		void setComputed() { m_computed = true; }
		bool isComputed() const { return m_computed; }

		void setResultSaved() { m_resultSaved = true; }
		bool isResultSaved() const { return m_resultSaved; }

		bool isShadowSignal() const { return m_isShadowSignal; }

		QString appSignalID() const { return m_signal->appSignalID(); }

		QUuid guid() const;

		Address16 ualAddr() const { return m_signal->ualAddr(); }
		Address16 regBufAddr() const { return m_signal->regBufAddr(); }

		E::SignalType signalType() const { return m_signal->signalType(); }
		E::AnalogAppSignalFormat analogSignalFormat() const { return m_signal->analogSignalFormat(); }
		int dataSize() const { return m_signal->dataSize(); }

		bool isAnalog() const { return m_signal->isAnalog(); }
		bool isDiscrete() const { return m_signal->isDiscrete(); }
		bool isBus() const { return m_signal->isBus(); }

		bool isAcquired() const { return m_signal->isAcquired(); }
		bool isInternal() const { return m_signal->isInternal(); }
		bool isInput() const { return m_signal->isInput(); }
		bool isOutput() const { return m_signal->isOutput(); }
		bool enableTuning() const { return m_signal->enableTuning(); }
		int ID() const { return m_signal->ID(); }

		bool isCompatibleDataFormat(const LogicAfbSignal& afbSignal) const;

		const Signal& constSignal() { return *m_signal; }

		Signal* signal() { return m_signal; }

		QString schemaID() const;

	private:
		Signal* m_signal = nullptr;							// pointer to signal in m_signalSet

		const AppItem* m_appItem = nullptr;					// application signals pointer (for real signals)
															// application functional block pointer (for shadow signals)
		QUuid m_guid;

		bool m_isShadowSignal = false;

		bool m_computed = false;
		bool m_resultSaved = false;
	};


	class AppSignalMap: public QObject, public HashedVector<QUuid, AppSignal*>
	{
		Q_OBJECT
	public:
		AppSignalMap(ModuleLogicCompiler& compiler);
		~AppSignalMap();

		bool insert(const AppItem* appItem);
		bool insert(const AppFb* appFb, const LogicPin& outputPin, IssueLogger* log);

		AppSignal* getSignal(const QString& appSignalID);
		bool containsSignal(const QString& appSignalID) const;

		void clear();

	private:
		QString getShadowSignalStrID(const AppFb* appFb, const LogicPin& outputPin);
		void incCounters(const AppSignal* appSignal);

	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;

		ModuleLogicCompiler& m_compiler;

		// counters for Internal signals only
		//
		int m_registeredAnalogSignalCount = 0;
		int m_registeredDiscreteSignalCount = 0;

		int m_notRegisteredAnalogSignalCount = 0;
		int m_notRegisteredDiscreteSignalCount = 0;
	};

}
