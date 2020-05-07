#include "UalItems.h"
#include "SignalSet.h"
#include "ModuleLogicCompiler.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// LogicAfb class implementation
	//
	// ---------------------------------------------------------------------------------------

	Afbl::Afbl(std::shared_ptr<Afb::AfbElement> afb) :
		m_afb(afb)
	{
		if (m_afb == nullptr)
		{
			assert(false);
			return;
		}
	}

	Afbl::~Afbl()
	{
	}

	bool Afbl::isBusProcessingAfb() const
	{
		switch(m_isBusProcessingAfb)
		{
		case -1:
			{
				// isBusProcessingAfbChecking() is not previously called
				bool isBusProcessingAfb = isBusProcessingAfbChecking();

				m_isBusProcessingAfb = (isBusProcessingAfb == false ? 0 : 1);

				return isBusProcessingAfb;
			}

		case 0:
			return false;

		case 1:
			return true;

		default:
			assert(false);			// disallowed value of m_isBusProcessingAfb
		}

		return false;
	}

	bool Afbl::isBusProcessingAfbChecking() const
	{
		const std::vector<Afb::AfbSignal>& inputSignals = afb().inputSignals();

		bool hasBusInputs = false;

		for(const Afb::AfbSignal& afbInSignal : inputSignals)
		{
			if (afbInSignal.type() == E::SignalType::Bus)
			{
				hasBusInputs = true;
				break;
			}

			// non-bus inputs bus processing afb - it is ok
		}

		const std::vector<Afb::AfbSignal>& outputSignals = afb().outputSignals();

		bool hasBusOutputs = false;
		bool hasNonBusOutputs = false;

		for(const Afb::AfbSignal& afbOutSignal : outputSignals)
		{
			if (afbOutSignal.type() == E::SignalType::Bus)
			{
				hasBusOutputs = true;
				continue;
			}

			hasNonBusOutputs = true;
		}

		if (hasBusInputs == true && hasBusOutputs == true)
		{
			if (hasNonBusOutputs == true)
			{
				// what should be doing with non-bus output???
				//
				assert(false);
				return false;
			}

			return true;
		}

		return false;
	}


	// ---------------------------------------------------------------------------------------
	//
	// AfbMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	AfblsMap::~AfblsMap()
	{
		clear();
	}

	bool AfblsMap::addInstance(UalAfb* ualAfb, IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);

		if (ualAfb == nullptr)
		{
			LOG_NULLPTR_ERROR(log);
			return false;
		}

		if (ualAfb->isSetFlagsItem() == true)
		{
			return true;
		}

		QString afbStrID = ualAfb->strID();

		Afbl* afbl = (*this).value(afbStrID, nullptr);

		if (afbl == nullptr)
		{
			LOG_INTERNAL_ERROR(log);			// unknown FBL strID
			return false;
		}

		int opCode = afbl->opCode();

		if (m_fblInstance.contains(opCode) == false)
		{
			// Unknown AFB type (opCode) (Logic schema %1, item %2).
			//
			log->errALC5129(ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
			return false;
		}

		int instance = -1;
		int maxInstances = afbl->maxInstances();

		QString instantiatorID = ualAfb->instantiatorID();

		if (afbl->hasRam() == false && m_nonRamFblInstance.contains(instantiatorID) == true)
		{
			// ualAfb has no RAM and its instantiatorID already exists - existing instance would be used
			//
			instance = m_nonRamFblInstance.value(instantiatorID);
		}
		else
		{
			// ualAfb has RAM or ualAfb has no RAM and its instantiatorID is not exist - new instance would be created

			instance = m_fblInstance[opCode];

			instance++;

			if (instance >= maxInstances)
			{
				// Max instances (%1) of AFB component '%2' is used (Logic schema %3, item %4)
				//
				log->errALC5130(maxInstances, afbl->componentCaption(), ualAfb->guid(), ualAfb->schemaID(), ualAfb->label());
				return false;
			}

			m_fblInstance[opCode] = instance;

			if (afbl->hasRam() == false)
			{
				// append new instantiatorID for non-RAM afb
				//
				m_nonRamFblInstance.insert(instantiatorID, instance);
			}
		}

		if (instance < 0)
		{
			assert(false);				// invalid instance number
			return false;
		}

		ualAfb->setInstance(instance);

		return true;
	}

	void AfblsMap::insert(std::shared_ptr<Afb::AfbElement> logicAfb)
	{
		if (logicAfb == nullptr)
		{
			assert(false);
			return;
		}

		if (contains(logicAfb->strID()))
		{
			assert(false);	// 	repeated guid
			return;
		}

		Afbl* afbl = new Afbl(logicAfb);

		HashedVector<QString, Afbl*>::insert(afbl->strID(), afbl);

		// initialize map Fbl opCode -> current instance
		//
		if (m_fblInstance.contains(logicAfb->opCode()) == false)
		{
			m_fblInstance.insert(logicAfb->opCode(), -1);			// init by -1, but used instances values is beginning from 0
		}

		// add AfbElement in/out signals to m_fblsSignals map
		//

		const std::vector<LogicAfbSignal>& inputSignals = logicAfb->inputSignals();

		for(LogicAfbSignal signal : inputSignals)
		{
			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = signal.operandIndex();

			if (m_afbSignals.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbSignals.insert(si, signal);
		}

		const std::vector<LogicAfbSignal>& outputSignals = logicAfb->outputSignals();

		for(LogicAfbSignal signal : outputSignals)
		{
			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = signal.operandIndex();

			if (m_afbSignals.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbSignals.insert(si, signal);
		}

		// add AfbElement params to m_fblsParams map
		//

		std::vector<LogicAfbParam>& params = logicAfb->params();

		for(LogicAfbParam param : params)
		{
			if (param.operandIndex() == UalAfb::FOR_USER_ONLY_PARAM_INDEX)
			{
				continue;
			}

			StrIDIndex si;

			si.strID = logicAfb->strID();
			si.index = param.operandIndex();

			if (m_afbParams.contains(si))
			{
				assert(false);
				continue;
			}

			m_afbParams.insert(si, &param);
		}
	}

	void AfblsMap::clear()
	{
		for(Afbl* afbl : *this)
		{
			if (afbl != nullptr)
			{
				delete afbl;
			}
		}

		HashedVector<QString, Afbl*>::clear();
	}

	const LogicAfbSignal AfblsMap::getAfbSignal(const QString& afbStrID, int signalIndex)
	{
		StrIDIndex si;

		si.strID = afbStrID;
		si.index = signalIndex;

		if (m_afbSignals.contains(si))
		{
			return m_afbSignals.value(si);
		}

		assert(false);

		return LogicAfbSignal();
	}

	int AfblsMap::getUsedInstances(int opCode) const
	{
		return m_fblInstance.value(opCode, -1) + 1;
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppItem class implementation
	//
	// ---------------------------------------------------------------------------------------

	const QString UalItem::SET_FLAGS_ITEM_CAPTION = "set_flags";
	const QString UalItem::SIM_LOCK_ITEM_CAPTION = "simlock";
	const QString UalItem::MISMATCH_ITEM_CAPTION = "mismatch";


	UalItem::UalItem()
	{
	}

	UalItem::UalItem(const UalItem& ualItem) :
		QObject()
	{
		m_appLogicItem = ualItem.m_appLogicItem;
		m_type = ualItem.m_type;
	}

	UalItem::UalItem(const AppLogicItem& appLogicItem) :
		m_appLogicItem(appLogicItem)
	{
	}

	UalItem::UalItem(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg)
	{
		init(afbElement, errorMsg);
	}

	bool UalItem::init(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg)
	{
		// m_appLogicItem.m_afbElement = *afbElement.get();

		m_appLogicItem.m_fblItem = std::shared_ptr<VFrame30::FblItemRect>(
					new VFrame30::SchemaItemAfb(VFrame30::SchemaUnit::Display, *afbElement.get(), &errorMsg));

		// copy parameters
		//
		for(Afb::AfbParam& param : afbElement->params())
		{
			m_appLogicItem.m_fblItem->toAfbElement()->setAfbParamByOpName(param.opName(), param.value());
		}

		return true;
	}

	QString UalItem::strID() const
	{
		if (m_appLogicItem.m_fblItem->isSignalElement() == true)
		{
			VFrame30::SchemaItemSignal* itemSignal= m_appLogicItem.m_fblItem->toSignalElement();

			if (itemSignal == nullptr)
			{
				assert(false);
				return "";
			}

			return itemSignal->appSignalIds();
		}

		if (m_appLogicItem.m_fblItem->isAfbElement() == true)
		{
			VFrame30::SchemaItemAfb* itemFb= m_appLogicItem.m_fblItem->toAfbElement();

			if (itemFb == nullptr)
			{
				assert(false);
				return "";
			}

			return itemFb->afbStrID();
		}

		if (m_appLogicItem.m_fblItem->isConstElement() == true)
		{
			VFrame30::SchemaItemConst* itemConst= m_appLogicItem.m_fblItem->toSchemaItemConst();

			if (itemConst == nullptr)
			{
				assert(false);
				return "";
			}

			return QString("Const(%1)").arg(itemConst->valueToString());
		}

		if (m_appLogicItem.m_fblItem->isLoopbackSourceElement() == true)
		{
			const VFrame30::SchemaItemLoopbackSource* loopbackSource= m_appLogicItem.m_fblItem->toLoopbackSourceElement();

			if (loopbackSource == nullptr)
			{
				assert(false);
				return "";
			}

			return loopbackSource->loopbackId();
		}

		assert(false);		// unknown type of item
		return "";
	}

	bool UalItem::isSetFlagsItem() const
	{
		if (isAfb() == false)
		{
			return false;
		}

		return m_appLogicItem.afbElement().caption() == SET_FLAGS_ITEM_CAPTION;
	}

	bool UalItem::isSimLockItem() const
	{
		if (isAfb() == false)
		{
			return false;
		}

		return m_appLogicItem.afbElement().caption() == SIM_LOCK_ITEM_CAPTION;
	}

	bool UalItem::isMismatchItem() const
	{
		if (isAfb() == false)
		{
			return false;
		}

		return m_appLogicItem.afbElement().caption().startsWith(MISMATCH_ITEM_CAPTION);
	}

	bool UalItem::assignFlags() const
	{
		const VFrame30::SchemaItemAfb* schemaItemAfb = dynamic_cast<const VFrame30::SchemaItemAfb*>(m_appLogicItem.m_fblItem.get());

		if (schemaItemAfb == nullptr)
		{
			assert(false);
			return false;
		}

		std::optional<bool> result = schemaItemAfb->getAssignFlagsValue();

		if (result.has_value() == true)
		{
			return result.value();
		}

		assert(false);			// AssignFlags property isn't exist in schemaItemAfb
								// Why assignFlags() has been called fot this schemaItemAfb ???
		return false;
	}

	E::UalItemType UalItem::type() const
	{
		if (m_type != E::UalItemType::Unknown)
		{
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isSignalElement() == true)
		{
			m_type = E::UalItemType::Signal;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isAfbElement() == true)
		{
			m_type = E::UalItemType::Afb;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isConstElement() == true)
		{
			m_type = E::UalItemType::Const;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isTransmitterElement() == true)
		{
			m_type = E::UalItemType::Transmitter;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isReceiverElement() == true)
		{
			m_type = E::UalItemType::Receiver;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isTerminatorElement() == true)
		{
			m_type = E::UalItemType::Terminator;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isBusComposerElement() == true)
		{
			m_type = E::UalItemType::BusComposer;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isBusExtractorElement() == true)
		{
			m_type = E::UalItemType::BusExtractor;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isLoopbackSourceElement() == true)
		{
			m_type = E::UalItemType::LoopbackSource;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isLoopbackTargetElement() == true)
		{
			m_type = E::UalItemType::LoopbackTarget;
			return m_type;
		}

		assert(false);

		m_type = E::UalItemType::Unknown;

		return m_type;
	}

	const LogicPin* UalItem::getPin(QUuid pinUuid) const
	{
		const std::vector<LogicPin>& inputPins = inputs();

		for(const LogicPin& inPin : inputPins)
		{
			if (inPin.guid() == pinUuid)
			{
				return &inPin;
			}
		}

		const std::vector<LogicPin>& outputPins = outputs();

		for(const LogicPin& outPin : outputPins)
		{
			if (outPin.guid() == pinUuid)
			{
				return &outPin;
			}
		}

		return nullptr;
	}

	const LogicPin* UalItem::getPin(const QString& pinCaption) const
	{
		const std::vector<LogicPin>& inputPins = inputs();

		for(const LogicPin& inPin : inputPins)
		{
			if (inPin.caption() == pinCaption)
			{
				return &inPin;
			}
		}

		const std::vector<LogicPin>& outputPins = outputs();

		for(const LogicPin& outPin : outputPins)
		{
			if (outPin.caption() == pinCaption)
			{
				return &outPin;
			}
		}

		return nullptr;
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFbParamValue class implementation
	//
	// ---------------------------------------------------------------------------------------

	AppFbParamValue::AppFbParamValue()
	{
	}

	AppFbParamValue::AppFbParamValue(const Afb::AfbParam& afbParam)
	{
		QVariant qv = afbParam.value();

		m_opName = afbParam.opName();
		m_caption = afbParam.caption();
		m_operandIndex = afbParam.operandIndex();
		m_instantiator = afbParam.instantiator();
		m_visible = afbParam.visible();

		if (afbParam.isDiscrete())
		{
			m_type = E::SignalType::Discrete;
			m_dataFormat = E::DataFormat::UnsignedInt;
			m_dataSize = 1;

			m_unsignedIntValue = qv.toUInt();
		}
		else
		{
			assert(afbParam.isAnalog());

			m_type = E::SignalType::Analog;
			m_dataSize = afbParam.size();

			switch(afbParam.dataFormat())
			{
			case E::DataFormat::SignedInt:
				m_dataFormat = E::DataFormat::SignedInt;
				m_signedIntValue = qv.toInt();
				break;

			case E::DataFormat::UnsignedInt:
				m_dataFormat = E::DataFormat::UnsignedInt;
				m_unsignedIntValue = qv.toUInt();
				break;

			case E::DataFormat::Float:
				assert(m_dataSize == SIZE_32BIT);
				m_dataFormat = E::DataFormat::Float;
				m_floatValue = qv.toFloat();
				break;

			default:
				assert(false);
			}
		}
	}

	quint32 AppFbParamValue::unsignedIntValue() const
	{
		assert(isUnsignedInt() == true);

		return m_unsignedIntValue;
	}

	void AppFbParamValue::setUnsignedIntValue(quint32 value)
	{
		assert(isUnsignedInt() == true);

		m_unsignedIntValue = value;
	}

	qint32 AppFbParamValue::signedIntValue() const
	{
		assert(isSignedInt32() == true);

		return m_signedIntValue;
	}

	void AppFbParamValue::setSignedIntValue(qint32 value)
	{
		assert(isSignedInt32() == true);

		m_signedIntValue = value;
	}

	float AppFbParamValue::floatValue() const
	{
		assert(isFloat32() == true);

		return static_cast<float>(m_floatValue);
	}

	void AppFbParamValue::setFloatValue(double value)
	{
		assert(isFloat32() == true);

		m_floatValue = value;
	}

	QString AppFbParamValue::toString() const
	{
		QString str;

		switch(m_dataFormat)
		{
		case E::DataFormat::UnsignedInt:
			str = QString("%1").arg(m_unsignedIntValue);
			break;

		case E::DataFormat::SignedInt:
			str = QString("%1").arg(m_signedIntValue);
			break;

		case E::DataFormat::Float:
			str = QString("%1").arg(m_floatValue);
			break;

		default:
			assert(false);
		}

		return str;
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFb class implementation
	//
	// ---------------------------------------------------------------------------------------

	const QString UalAfb::IN_PIN_CAPTION("in");
	const QString UalAfb::OUT_PIN_CAPTION("out");

	const QString UalAfb::IN_1_PIN_CAPTION("in_1");
	const QString UalAfb::IN_2_PIN_CAPTION("in_2");
	const QString UalAfb::IN_3_PIN_CAPTION("in_3");
	const QString UalAfb::IN_4_PIN_CAPTION("in_4");

	const QString UalAfb::OUT_1_PIN_CAPTION("out_1");
	const QString UalAfb::OUT_2_PIN_CAPTION("out_2");
	const QString UalAfb::OUT_3_PIN_CAPTION("out_3");
	const QString UalAfb::OUT_4_PIN_CAPTION("out_4");

	const QString UalAfb::SIMLOCK_SIM_PIN_CAPTION("sim");
	const QString UalAfb::SIMLOCK_BLOCK_PIN_CAPTION("block");

	const QString UalAfb::VALIDITY_PIN_CAPTION("validity");
	const QString UalAfb::SIMULATED_PIN_CAPTION("simulated");
	const QString UalAfb::BLOCKED_PIN_CAPTION("blocked");
	const QString UalAfb::MISMATCH_PIN_CAPTION("mismatch");
	const QString UalAfb::HIGH_LIMIT_PIN_CAPTION("high_limit");
	const QString UalAfb::LOW_LIMIT_PIN_CAPTION("low_limit");

	UalAfb::UalAfb(const UalItem& appItem, bool isBusProcessingAfb) :
		UalItem(appItem),
		m_isBusProcessing(isBusProcessingAfb)
	{
		// initialize m_paramValuesArray
		//
		for(const Afb::AfbParam& afbParam : appItem.params())
		{
			AppFbParamValue value(afbParam);

			m_paramValuesArray.insert(afbParam.opName(), value);
		}
	}

	bool UalAfb::isConstComaparator() const
	{
		return opcode() == CONST_COMPARATOR_OPCODE;
	}

	bool UalAfb::isDynamicComaparator() const
	{
		return opcode() == DYNAMIC_COMPARATOR_OPCODE;
	}

	bool UalAfb::isComparator() const
	{
		quint16 oc = opcode();

		return oc ==  CONST_COMPARATOR_OPCODE || oc == DYNAMIC_COMPARATOR_OPCODE;
	}

	bool UalAfb::isBusProcessing() const
	{
		return m_isBusProcessing;
	}

	QString UalAfb::instantiatorID() const
	{
		if (m_instantiatorID.isEmpty() == false)
		{
			return m_instantiatorID;
		}

		m_instantiatorID = QString("opCode:%1").arg(afb().opCode());

		bool firstParam = true;

		// append instantiator param's values to instantiatorID
		//
		for(const AppFbParamValue& paramValue : m_paramValuesArray)
		{
			if (paramValue.instantiator() == false)
			{
				continue;
			}

			if (firstParam == true)
			{
				m_instantiatorID += ":params";
				firstParam = false;
			}

			switch(paramValue.dataFormat())
			{
			case E::DataFormat::Float:
				m_instantiatorID += QString(":%1").arg(paramValue.floatValue());
				break;

			case E::DataFormat::SignedInt:
				m_instantiatorID += QString(":%1").arg(paramValue.signedIntValue());
				break;

			case E::DataFormat::UnsignedInt:
				m_instantiatorID += QString(":%1").arg(paramValue.unsignedIntValue());
				break;

			default:
				assert(false);
			}
		}

		return m_instantiatorID;
	}

	bool UalAfb::getAfbParamByIndex(int index, LogicAfbParam* afbParam) const
	{
		for(const LogicAfbParam& param : afb().params())
		{
			if (param.operandIndex() == index)
			{
				*afbParam = param;
				return true;
			}
		}

		LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
				  QString(tr("Not found parameter with opIndex = %1 in FB %2")).arg(index).arg(caption()));

		return false;
	}


	bool UalAfb::getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const
	{
		if (afbSignal == nullptr)
		{
			return false;
		}

		for(const LogicAfbSignal& input : afb().inputSignals())
		{
			if (input.operandIndex() == index)
			{
				*afbSignal = input;
				return true;
			}
		}

		for(const LogicAfbSignal& output : afb().outputSignals())
		{
			if (output.operandIndex() == index)
			{
				*afbSignal = output;
				return true;
			}
		}

		LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
				  QString(tr("Not found signal with opIndex = %1 in FB %2")).arg(index).arg(caption()));

		return false;
	}

	bool UalAfb::getAfbSignalByPinUuid(QUuid pinUuid, LogicAfbSignal* afbSignal) const
	{
		if (afbSignal == nullptr)
		{
			return false;
		}

		for(const LogicPin& inPin : inputs())
		{
			if (inPin.guid() == pinUuid)
			{
				return getAfbSignalByPin(inPin, afbSignal);
			}
		}

		for(const LogicPin& outPin : outputs())
		{
			if (outPin.guid() == pinUuid)
			{
				return getAfbSignalByPin(outPin, afbSignal);
			}
		}

		LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
				  QString(tr("Not found signal with pin Uuid = %1 in FB %2")).arg(pinUuid.toString()).arg(caption()));

		return false;
	}

	bool UalAfb::checkRequiredParameters(const QStringList& requiredParams)
	{
		return checkRequiredParameters(requiredParams, true);
	}

	bool UalAfb::checkRequiredParameters(const QStringList& requiredParams, bool displayError)
	{
		bool result = true;

		for(const QString& opName : requiredParams)
		{
			result &= checkRequiredParameter(opName, displayError);
		}

		return result;
	}

	bool UalAfb::checkRequiredParameter(const QString& requiredParam, bool displayError)
	{
		if (m_paramValuesArray.contains(requiredParam) == false)
		{
			if (displayError == true)
			{
				// Required parameter '%1' of AFB '%2' is missing.
				//
				m_log->errALC5045(requiredParam, caption(), guid());
			}

			return false;
		}

		return true;
	}

	bool UalAfb::checkUnsignedInt(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type Unsigned Int.
		//
		m_log->errALC5046(paramValue.opName(), caption(), guid());

		return false;
	}

	bool UalAfb::checkUnsignedInt16(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt16())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type 16-bit Unsigned Int.
		//
		m_log->errALC5047(paramValue.opName(), caption(), guid());

		return false;
	}

	bool UalAfb::checkUnsignedInt32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isUnsignedInt32())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type 32-bit Unsigned Int.
		//
		m_log->errALC5048(paramValue.opName(), caption(), guid());

		return false;
	}

	bool UalAfb::checkSignedInt32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isSignedInt32())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type 32-bit Signed Int.
		//
		m_log->errALC5049(paramValue.opName(), caption(), guid());

		return false;
	}

	bool UalAfb::checkFloat32(const AppFbParamValue& paramValue)
	{
		if (paramValue.isFloat32())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type 32-bit Float.
		//
		m_log->errALC5050(paramValue.opName(), caption(), guid());

		return false;
	}

	int UalAfb::lmDescriptionNumber() const
	{
		if (m_compiler == nullptr)
		{
			assert(false);
			return 0;
		}

		return m_compiler->lmDescriptionNumber();
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFbMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	UalAfbsMap::~UalAfbsMap()
	{
		clear();
	}

	UalAfb* UalAfbsMap::insert(UalAfb *appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return nullptr;
		}

		appFb->setNumber(m_fbNumber);

		m_fbNumber++;

		HashedVector<QUuid, UalAfb*>::insert(appFb->guid(), appFb);

		return appFb;
	}


	void UalAfbsMap::clear()
	{
		for(UalAfb* appFb : *this)
		{
			delete appFb;
		}

		HashedVector<QUuid, UalAfb*>::clear();
	}

	// ---------------------------------------------------------------------------------------
	//
	// UalSignal class implementation
	//
	// ---------------------------------------------------------------------------------------

	const QString UalSignal::AUTO_CONST_SIGNAL_ID_PREFIX("#AUTO_CONST");
	const QString UalSignal::AUTO_SIGNAL_ID_PREFIX("#AUTO_SIGNAL");
	const QString UalSignal::AUTO_BUS_ID_PREFIX("#AUTO_BUS");

	UalSignal::UalSignal()
	{
	}

	UalSignal::~UalSignal()
	{
		if (m_optoSignalNativeCopy != nullptr)
		{
			delete m_optoSignalNativeCopy;
		}

		m_refSignals.clear();
	}

	bool UalSignal::createRegularSignal(const UalItem* ualItem, Signal* s)
	{
		// ualItem can be == nullptr!!!
		//
		m_ualItem = ualItem;

		if (s == nullptr )
		{
			assert(false);
			return false;
		}

		appendRefSignal(s, false);

		// input and tuning signals have already been computed
		//
		if (isSource() == true)
		{
			setComputed();
		}

		return true;
	}

	bool UalSignal::createConstSignal(const UalItem* ualItem,
										const QString& constSignalID,
										E::SignalType constSignalType,
										E::AnalogAppSignalFormat constAnalogFormat,
										Signal** autoSignalPtr)
	{
		TEST_PTR_RETURN_FALSE(ualItem);
		TEST_PTR_RETURN_FALSE(autoSignalPtr);

		m_ualItem = ualItem;

		const UalConst* ualConst = ualItem->ualConst();

		TEST_PTR_RETURN_FALSE(ualConst);

		// const UalSignal creation

		Signal* autoSignal = *autoSignalPtr = new Signal;

		autoSignal->setAppSignalID(constSignalID);
		autoSignal->setCustomAppSignalID(QString(constSignalID).remove("#"));
		autoSignal->setCaption(autoSignal->customAppSignalID());

		autoSignal->setSignalType(constSignalType);
		autoSignal->setInOutType(E::SignalInOutType::Internal);
		autoSignal->setAnalogSignalFormat(constAnalogFormat);

		switch(constSignalType)
		{
		case E::SignalType::Discrete:
			autoSignal->setDataSize(SIZE_1BIT);
			break;

		case E::SignalType::Analog:
			assert(constAnalogFormat == E::AnalogAppSignalFormat::Float32 || constAnalogFormat == E::AnalogAppSignalFormat::SignedInt32);
			autoSignal->setDataSize(SIZE_32BIT);
			break;

		default:
			assert(false);
		}

		autoSignal->setAcquire(false);

		appendRefSignal(autoSignal, false);

		// set Const signal fields

		m_isConst = true;
		m_constDiscreteValue = ualConst->discreteValue();
		m_constIntValue = ualConst->intValue();
		m_constFloatValue = ualConst->floatValue();

		setComputed();

		return true;
	}

	bool UalSignal::createAutoSignal(const UalItem* ualItem,
									const QString& signalID,
									E::SignalType signalType,
									E::AnalogAppSignalFormat analogFormat,
									Signal** autoSignalPtr)
	{
		TEST_PTR_RETURN_FALSE(ualItem);
		TEST_PTR_RETURN_FALSE(autoSignalPtr);

		m_ualItem = ualItem;

		// analog or discrete auto UalSignal creation

		Signal* autoSignal = *autoSignalPtr = new Signal;

		autoSignal->setAppSignalID(signalID);
		autoSignal->setCustomAppSignalID(QString(signalID).remove("#"));
		autoSignal->setCaption(autoSignal->customAppSignalID());

		autoSignal->setSignalType(signalType);
		autoSignal->setInOutType(E::SignalInOutType::Internal);
		autoSignal->setAnalogSignalFormat(analogFormat);

		switch(signalType)
		{
		case E::SignalType::Discrete:
			autoSignal->setDataSize(SIZE_1BIT);
			break;

		case E::SignalType::Analog:
			assert(analogFormat == E::AnalogAppSignalFormat::Float32 || analogFormat == E::AnalogAppSignalFormat::SignedInt32);
			autoSignal->setDataSize(SIZE_32BIT);
			break;

		case E::SignalType::Bus:
		default:
			assert(false);
		}

		autoSignal->setAcquire(false);

		appendRefSignal(autoSignal, false);

		return true;
	}

	bool UalSignal::createBusParentSignal(const UalItem* ualItem,
											Signal* busSignal,
											Builder::BusShared bus,
											const QString& outPinCaption,
											std::shared_ptr<Hardware::DeviceModule> lm,
											Signal** autoSignalPtr)
	{
		// create parent Bus signal
		//
		if (ualItem == nullptr || bus == nullptr)
		{
			assert(false);
			return false;
		}

		m_ualItem = ualItem;
		m_bus = bus;

		if (busSignal == nullptr)
		{
			// create auto bus signal
			//
			QString appSignalID = QString("%1_%2_%3_%4").
										arg(AUTO_BUS_ID_PREFIX).
										arg(lm->equipmentIdTemplate()).
										arg(ualItem->label()).
										arg(outPinCaption.toUpper());

			busSignal = *autoSignalPtr = new Signal;

			busSignal->setAppSignalID(appSignalID);
			busSignal->setCustomAppSignalID(appSignalID.remove("#"));
			busSignal->setCaption(busSignal->customAppSignalID());

			busSignal->setSignalType(E::SignalType::Bus);
			busSignal->setBusTypeID(bus->busTypeID());

			busSignal->setDataSizeW(bus->sizeW());

			busSignal->setAcquire(false);

			busSignal->setLm(lm);
		}
		else
		{
			assert(busSignal->isBus());
			assert(busSignal->busTypeID() == bus->busTypeID());
		}

		appendRefSignal(busSignal, false);

		return true;
	}

	bool UalSignal::appendRefSignal(Signal* s, bool isOptoSignal)
	{
		if (s == nullptr)
		{
			assert(false);
			return false;
		}

		for(Signal* pesentSignal : m_refSignals)
		{
			if (pesentSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (pesentSignal == s)
			{
				assert(false);			// not duplicate ref, why?
				return true;
			}
		}

		if (m_refSignals.count() > 0)
		{
			// check signals compatibility
			//
			Signal* first = m_refSignals[0];

			if (first->signalType() != s->signalType())
			{
				assert(false);
				return false;
			}

			if (first->dataSize() != s->dataSize())
			{
				assert(false);
				return false;
			}

			if (first->isAnalog() == true)
			{
				if (first->analogSignalFormat() != s->analogSignalFormat())
				{
					assert(false);
					return false;
				}

				if (first->byteOrder() != s->byteOrder())
				{
					assert(false);
					return false;
				}
			}

			if (first->isBus() == true)
			{
				if (first->busTypeID() != s->busTypeID())
				{
					assert(false);
					return false;
				}
			}

			if (isSource() == true &&
				(s->isInput() == true || s->enableTuning() == true || isOptoSignal == true))
			{
				// only one Source signal in m_signals[] can be exists
				//
				assert(false);
				return false;
			}
		}

		m_refSignals.append(s);

		// In UalSignal, Input, Tunable and Opto signals treat as Source

		m_isInput |= s->isInput();
		m_isTunable |= s->enableTuning();
		m_isOptoSignal |= isOptoSignal;

		// UalSignal can be Input and Output simultaneously (also as Tunable and Output, OptoSignal and Output)
		// for example, if Input Signal directly connected to Output Signal (or Tunable => Output, OptoSignal => Output)
		// in this case m_ualAddress set to Sourcet signal ioBufAddr and memory for that signal is not allocate (used ioBuf memory)
		// value of Source signal can't be changed by UAL

		m_isOutput |= s->isOutput();

		m_isAcquired |= s->isAcquired();

		m_refSignalsIDs = refSignalIDsJoined();

		return true;
	}

	bool UalSignal::appendBusChildRefSignals(const QString& busSignalID, Signal* s)
	{
		UalSignal* childSignal = m_busChildSignals.value(busSignalID, nullptr);

		if (childSignal == nullptr)
		{
			assert(false);
			return false;
		}

		return childSignal->appendRefSignal(s, false);
	}

	Address16 UalSignal::ioBufAddr()
	{
		if (m_isInput == true)
		{
			Signal* inSignal = getInputSignal();

			if (inSignal == nullptr)
			{
				assert(false);
				return Address16();
			}

			return inSignal->ioBufAddr();
		}

		if (m_isTunable == true)
		{
			Signal* tunSignal = getTunableSignal();

			if (tunSignal == nullptr)
			{
				assert(false);
				return Address16();
			}

			return tunSignal->ioBufAddr();
		}

		if (m_isOptoSignal == true)
		{
			Signal* s = signal();

			if (s == nullptr)
			{
				assert(false);
				return Address16();
			}

			return s->ioBufAddr();
		}

		return Address16();
	}

	Signal* UalSignal::signal() const
	{
		if (m_refSignals.count() < 1)
		{
			assert(false);
			return nullptr;
		}

		return m_refSignals[0];
	}

	bool UalSignal::isCompatible(const Signal* s, IssueLogger* log) const
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(s, log);

		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			assert(false);
			return false;
		}

		return m_refSignals[0]->isCompatibleFormat(*s);
	}

	bool UalSignal::isCompatible(const UalItem& ualItem, const LogicAfbSignal& afbSignal, IssueLogger* log) const
	{
		TEST_PTR_RETURN_FALSE(log);

		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			assert(false);
			return false;
		}

		if (afbSignal.isBus() == true)
		{
			if(isBus() == true)
			{
				// bus signal connection to bus input checking
				//
				if (m_bus == nullptr)
				{
					assert(false);
					return false;
				}

				switch(afbSignal.busDataFormat())
				{
				case E::BusDataFormat::Discrete:

					if (m_bus->busDataFormat() == E::BusDataFormat::Discrete)
					{
						return true;
					}

					// Non-discrete busses is not allowed on input '%1'. (Item %2, logic schema %3).
					//
					log->errALC5172(afbSignal.caption(), ualItem.label(), ualItem.guid(), ualItem.schemaID());

					return false;

				case E::BusDataFormat::Mixed:
					// any bus can be connected to this afbSignal
					//
					return true;

				default:
					LOG_INTERNAL_ERROR_MSG(log, "Unknown E::BusDataFormat");
				}

				return false;
			}

			if (isDiscrete() == true)
			{
				// discrete signal connection to bus input checking
				//
				switch(afbSignal.busDataFormat())
				{
				case E::BusDataFormat::Discrete:
				case E::BusDataFormat::Mixed:
					return true;

				default:
					LOG_INTERNAL_ERROR_MSG(log, "Unknown E::BusDataFormat");
				}
			}

			return false;
		}

		return m_refSignals[0]->isCompatibleFormat(afbSignal.type(), afbSignal.dataFormat(), afbSignal.size(), afbSignal.byteOrder());
	}

	bool UalSignal::isCompatible(const Builder::BusSignal& busSignal, IssueLogger* log) const
	{
		TEST_PTR_RETURN_FALSE(log);

		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			assert(false);
			return false;
		}

		switch(busSignal.signalType)
		{
		case E::SignalType::Analog:
		case E::SignalType::Discrete:
			return m_refSignals[0]->isCompatibleFormat(busSignal.signalType, busSignal.analogFormat, E::ByteOrder::BigEndian);

		case E::SignalType::Bus:
			return m_refSignals[0]->isCompatibleFormat(busSignal.signalType, busSignal.busTypeID);

		default:
			assert(false);
		}

		return false;
	}

	bool UalSignal::isCompatible(const UalSignal* ualSignal, IssueLogger* log) const
	{
		return isCompatible(ualSignal->signal(), log);
	}

	void UalSignal::setReceivedOptoAppSignalID(const QString& recvAppSignalID)
	{
		m_receivedOptoAppSignalID = recvAppSignalID;
		m_isOptoSignal = true;
	}

	void UalSignal::setLoopback(std::shared_ptr<Loopback> loopback)
	{
		if (m_loopback != nullptr)
		{
			assert(false);				// reassigning of loopback, why?
		}

		m_loopback = loopback;
	}

	std::shared_ptr<Loopback> UalSignal::loopback() const
	{
		return m_loopback;
	}

	QString UalSignal::loopbackID() const
	{
		if (m_loopback == nullptr)
		{
			assert(false);
			return QString();
		}

		return m_loopback->loopbackID();
	}

	E::SignalType UalSignal::constType() const
	{
		assert(m_isConst == true);

		return m_refSignals[0]->signalType();
	}

	E::AnalogAppSignalFormat UalSignal::constAnalogFormat() const
	{
		assert(m_isConst == true);
		assert(constType() == E::SignalType::Analog);

		return m_refSignals[0]->analogSignalFormat();
	}

	int UalSignal::constDiscreteValue() const
	{
		assert(m_isConst == true);
		assert(constType() == E::SignalType::Discrete);

		return m_constDiscreteValue == 0 ? 0 : 1;
	}

	int UalSignal::constAnalogIntValue() const
	{
		assert(m_isConst == true);
		assert(constAnalogFormat() == E::AnalogAppSignalFormat::SignedInt32);

		return m_constIntValue;
	}

	float UalSignal::constAnalogFloatValue() const
	{
		assert(m_isConst == true);
		assert(constAnalogFormat() == E::AnalogAppSignalFormat::Float32);

		return static_cast<float>(m_constFloatValue);
	}

	double UalSignal::constValue() const
	{
		if (m_isConst == false)
		{
			assert(false);
			return 0;
		}

		double constVal = 0;

		switch(constType())
		{
		case E::SignalType::Discrete:
			constVal = static_cast<double>(constDiscreteValue());
			break;

		case E::SignalType::Analog:

			switch(constAnalogFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				constVal = static_cast<double>(constAnalogFloatValue());
				break;

			case E::AnalogAppSignalFormat::SignedInt32:
				constVal = static_cast<double>(constAnalogIntValue());
				break;

			default:
				assert(false);
			}
			break;

		default:
			assert(false);
		}

		return constVal;
	}

	double UalSignal::constValueIfConst() const
	{
		if (m_isConst == false)
		{
			return 0;
		}

		return constValue();
	}

	bool UalSignal::setUalAddr(Address16 ualAddr)
	{
		if (m_isConst == true)
		{
			assert(false);					// for Const signals ualAddr isn't assigned
			return false;
		}

		assert(ualAddr.isValid() == true);

		if (m_ualAddr.isValid() == true && m_isBusChild == true)
		{
			return true;			// ualAddress of bus child signal is allredy set, its ok
		}

		if (m_ualAddr.isValid() == true && m_isBusChild == false)
		{
			assert(false);				// why and where m_ualAddr is already set???
			return false;
		}

		m_ualAddr = ualAddr;

		// set same ual address for all associated signals

		for(Signal* s : m_refSignals)
		{
			assert(s->ualAddr().isValid() == false);

			s->setUalAddr(ualAddr);
		}

		if (isBus() == false)
		{
			return true;
		}

		if (m_bus == nullptr)
		{
			assert(false);				// m_bus can't be null
			return false;
		}

		assert(ualAddr.bit() == 0);		// bus must be aligned to word

		bool result = true;

		for(const BusSignal& busSignal : m_bus->busSignals())
		{
			UalSignal* childSignal = m_busChildSignals.value(busSignal.signalID);

			if (childSignal == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			int busBitAddr = ualAddr.bitAddress();
			int busSignalBitAddr = busSignal.inbusAddr.bitAddress();

			Address16 addr(0, 0);

			addr.addBit(busBitAddr + busSignalBitAddr);

			result &= childSignal->setUalAddr(addr);
		}

		return result;
	}

	bool UalSignal::setRegBufAddr(Address16 regBufAddr)
	{
		assert(regBufAddr.isValid() == true);

		if (m_regBufAddr.isValid() == true)
		{
			assert(false);				// m_regBufAddr is already set
			return false;
		}

		m_regBufAddr = regBufAddr;

		// set same regBufAddr for all associated acquired signals

		for(Signal* s : m_refSignals)
		{
			if (s->isAcquired() == false)
			{
				continue;
			}

			assert(s->regBufAddr().isValid() == false);

			s->setRegBufAddr(regBufAddr);
		}

		return true;
	}

	bool UalSignal::setRegValueAddr(Address16 regValueAddr)
	{
		assert(regValueAddr.isValid() == true);

		if (m_regValueAddr.isValid() == true)
		{
			assert(false);				// m_regValueAddr is already set
			return false;
		}

		m_regValueAddr = regValueAddr;

		// set same regBufAddr for all associated acquired signals

		for(Signal* s : m_refSignals)
		{
			if (s->isAcquired() == false)
			{
				continue;
			}

			assert(s->regValueAddr().isValid() == false);

			s->setRegValueAddr(regValueAddr);
		}

		return true;
	}

	void UalSignal::sortRefSignals()
	{
		// sorting m_refSignals by appSignalID ascending
		//
		int count = m_refSignals.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (m_refSignals[i]->appSignalID() > m_refSignals[k]->appSignalID())
				{
					Signal* tmp = m_refSignals[i];
					m_refSignals[i] = m_refSignals[k];
					m_refSignals[k] = tmp;
				}
			}
		}
	}

	Signal* UalSignal::getInputSignal()
	{
		Signal* inputSignal = nullptr;

		for(Signal* s : m_refSignals)
		{
			if (s->isInput() == true)
			{
				inputSignal = s;
				break;
			}
		}

		return inputSignal;
	}

	Signal* UalSignal::getTunableSignal()
	{
		Signal* tunableSignal = nullptr;

		for(Signal* s : m_refSignals)
		{
			if (s->enableTuning() == true)
			{
				tunableSignal = s;
				break;
			}
		}

		return tunableSignal;
	}

	QVector<Signal*> UalSignal::getAnalogOutputSignals()
	{
		QVector<Signal*> analogOutputs;

		if (isAnalog() == false)
		{
			assert(false);
			return analogOutputs;
		}

		for(Signal* s : m_refSignals)
		{
			assert(s->isAnalog() == true);

			if (s->isOutput() == true)
			{
				analogOutputs.append(s);
			}
		}

		return analogOutputs;
	}

	QStringList UalSignal::refSignalIDs() const
	{
		QStringList list;

		for(Signal* s : m_refSignals)
		{
			list.append(s->appSignalID());
		}

		return list;
	}

	void UalSignal::refSignalIDs(QStringList* appSignalIDs) const
	{
		if (appSignalIDs == nullptr)
		{
			assert(false);
			return;
		}

		appSignalIDs->clear();

		for(Signal* s : m_refSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			appSignalIDs->append(s->appSignalID());
		}
	}

	QString UalSignal::refSignalIDsJoined() const
	{
		QStringList ids;

		refSignalIDs(&ids);

		return ids.join(", ");
	}

	QString UalSignal::refSignalIDsJoined(const QString& separator) const
	{
		QStringList ids;

		refSignalIDs(&ids);

		return ids.join(separator);
	}

	QStringList UalSignal::acquiredRefSignalsIDs() const
	{
		QStringList list;

		for(Signal* s : m_refSignals)
		{
			if (s->isAcquired() == false)
			{
				continue;
			}

			list.append(s->appSignalID());
		}

		return list;
	}

	QString UalSignal::optoConnectionID() const
	{
		if (m_isOptoSignal == false)
		{
			assert(false);
			return QString();
		}

		if (m_ualItem == nullptr)
		{
			assert(false);
			return QString();
		}

		const UalReceiver* ualReceiver = m_ualItem->ualReceiver();

		if (ualReceiver == nullptr)
		{
			assert(false);
			return QString();
		}

		return ualReceiver->connectionIds();
	}

	void UalSignal::setUalItem(const UalItem* ualItem)
	{
		if (m_ualItem == nullptr)
		{
			m_ualItem = ualItem;
		}
	}

	QUuid UalSignal::ualItemGuid() const
	{
		if (m_ualItem != nullptr)
		{
			return m_ualItem->guid();
		}

		return QUuid();
	}

	QString UalSignal::ualItemSchemaID() const
	{
		if (m_ualItem != nullptr)
		{
			return m_ualItem->schemaID();
		}

		return QString();
	}

	QString UalSignal::ualItemLabel() const
	{
		if (m_ualItem != nullptr)
		{
			return m_ualItem->label();
		}

		return QString();
	}

	bool UalSignal::appendBusChildSignal(const QString& busSignalID, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		if (m_busChildSignals.contains(busSignalID) == true)
		{
			assert(false);
			return false;
		}

		ualSignal->setBusChild(true);

		m_busChildSignals.insert(busSignalID, ualSignal);

		return true;
	}

	UalSignal* UalSignal::getBusChildSignal(const QString& busSignalID)
	{
		if (isBus() == false)
		{
			assert(false);
			return nullptr;
		}

		return m_busChildSignals.value(busSignalID, nullptr);
	}

	void UalSignal::setAcquired(bool acquired)
	{
		m_isAcquired = acquired;

		for(Signal* refSignal : m_refSignals)
		{
			TEST_PTR_CONTINUE(refSignal);

			refSignal->setAcquire(acquired);
		}
	}

	bool UalSignal::addStateFlagSignal(const QString& signalWithFlagID, E::AppSignalStateFlagType flagType, const QString& flagSignalID, IssueLogger* log)
	{
		bool result = true;

		bool signalWithFlagID_isFound = false;

		for(Signal* s : m_refSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->appSignalID() != signalWithFlagID)
			{
				continue;
			}

			signalWithFlagID_isFound = true;

			bool res = s->addFlagSignalID(flagType, flagSignalID);

			if (res == false)
			{
				// Duplicate assigning of signal %1 to flag %2 of signal %3. Signal %4 already assigned to this flag.
				//
				log->errALC5168(flagSignalID,
								E::valueToString<E::AppSignalStateFlagType>(flagType),
								s->appSignalID(),
								s->getFlagSignalID(flagType),
								QUuid(),
								QString());
			}

			result &= res;
		}

		if (signalWithFlagID_isFound == false)
		{
			LOG_INTERNAL_ERROR_MSG(log,
								   QString("UalSignal::addStateFlagSignal error. SignalWithFlagID %1 is not found in UalSignal %2 to assigninf flag signal %3").
										arg(signalWithFlagID).arg(refSignalIDsJoined()).arg(flagSignalID));
			result = false;
		}

		return result;
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppSignalsMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	UalSignalsMap::UalSignalsMap(ModuleLogicCompiler& compiler, IssueLogger* log) :
		m_compiler(compiler),
		m_log(log),
		m_discreteSignalsHeap(SIZE_1BIT, compiler.generateExtraDebugInfo(), log),
		m_analogAndBusSignalsHeap(SIZE_16BIT, compiler.generateExtraDebugInfo(), log)
	{
	}

	UalSignalsMap::~UalSignalsMap()
	{
		clear();
	}

	UalSignal* UalSignalsMap::createSignal(Signal* s)
	{
		return createSignal(nullptr, s, QUuid());
	}

	UalSignal* UalSignalsMap::createSignal(const UalItem* ualItem, Signal* s, QUuid outPinUuid)
	{
		// ualItem can be nullptr!!!

		if (s == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		UalSignal* ualSignal = m_idToSignalMap.value(s->appSignalID(), nullptr);

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			ualSignal->setUalItem(ualItem);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		// create new signal
		//
		ualSignal = new UalSignal;

		bool result = ualSignal->createRegularSignal(ualItem, s);

		if (result == false)
		{
			delete ualSignal;
			return nullptr;
		}

		result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			delete ualSignal;
			return nullptr;
		}

		return ualSignal;
	}

	UalSignal* UalSignalsMap::createConstSignal(const UalItem* ualItem,
												E::SignalType constSignalType,
												E::AnalogAppSignalFormat constAnalogFormat,
												QUuid outPinUuid)
	{
		if (ualItem == nullptr)
		{
			assert(false);
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		QString constSignalID = QString("%1_%2_%3").
										arg(UalSignal::AUTO_CONST_SIGNAL_ID_PREFIX).
										arg(m_compiler.lmEquipmentID()).
										arg(ualItem->label());

		UalSignal* ualSignal = m_idToSignalMap.value(constSignalID, nullptr);

		if (ualSignal != nullptr)
		{
			// const already in map
			//
			assert(false);

			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		const UalConst* ualConst = ualItem->ualConst();

		if (ualConst == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		// create new const signal
		//
		ualSignal = new UalSignal;

		Signal* autoSignalPtr = nullptr;

		bool result = ualSignal->createConstSignal(ualItem,
									  constSignalID,
									  constSignalType,
									  constAnalogFormat,
									  &autoSignalPtr);
		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete ualSignal;
			return nullptr;
		}

		result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete ualSignal;
			return nullptr;
		}

		if (autoSignalPtr != nullptr)
		{
			m_compiler.signalSet()->append(autoSignalPtr);
		}
		else
		{
			assert(false);
		}

		return ualSignal;
	}

	UalSignal* UalSignalsMap::createAutoSignal(const UalItem* ualItem, QUuid outPinUuid,
											   const LogicAfbSignal& templateOutAfbSignal,
											   std::optional<int> expectedReadCount)
	{
		if (ualItem == nullptr)
		{
			assert(false);
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		E::AnalogAppSignalFormat analogFormat = E::AnalogAppSignalFormat::SignedInt32;

		bool result = getAnalogFormat(templateOutAfbSignal, &analogFormat);

		if (result == false)
		{
			// Format of AFB signal %1 is not compatible with any known application signals format
			//
			m_log->errALC5179(ualItem->caption(), templateOutAfbSignal.caption(), ualItem->guid(), ualItem->schemaID());

			return nullptr;
		}

		return privateCreateAutoSignal(ualItem, outPinUuid, templateOutAfbSignal.type(), analogFormat, expectedReadCount);
	}

	UalSignal* UalSignalsMap::createAutoSignal(const UalItem* ualItem, QUuid outPinUuid, const Signal& templateSignal)
	{
		return privateCreateAutoSignal(ualItem, outPinUuid, templateSignal.signalType(), templateSignal.analogSignalFormat(), -1);
	}

	UalSignal* UalSignalsMap::createBusParentSignal(const UalItem* ualItem,
													Signal* connectedSignal,
													BusShared bus,
													QUuid outPinUuid,
													const QString& outPinCaption,
													std::shared_ptr<Hardware::DeviceModule> lm)
	{
		if (ualItem == nullptr)
		{
			Q_ASSERT(false);
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		if ((connectedSignal != nullptr && connectedSignal->isBus() == false) || (bus == nullptr))
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		// connectedSignal can bee nullptr!!!
		//
		if (connectedSignal != nullptr)
		{
			UalSignal* ualSignal = m_idToSignalMap.value(connectedSignal->appSignalID(), nullptr);

			if (ualSignal != nullptr)
			{
				// signal already in map
				//
				assert(m_pinToSignalMap.contains(outPinUuid) == false);

				appendPinRefToSignal(outPinUuid, ualSignal);

				return ualSignal;
			}
		}

		UalSignal* busParentSignal = new UalSignal;

		Signal* autoSignalPtr = nullptr;

		bool result = busParentSignal->createBusParentSignal(ualItem, connectedSignal, bus, outPinCaption, lm, &autoSignalPtr);

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete busParentSignal;
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		result = insertNew(outPinUuid, busParentSignal);

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete busParentSignal;
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		const QVector<BusSignal>& busSignals = bus->busSignals();

		for(const BusSignal& busSignal : busSignals)
		{
			Signal* sChild = m_compiler.signalSet()->appendBusChildSignal(*busParentSignal->signal(), bus, busSignal);

			UalSignal* busChildSignal = nullptr;

			switch(busSignal.signalType)
			{
			case E::SignalType::Analog:
			case E::SignalType::Discrete:
				busChildSignal = createSignal(sChild);
				break;

			case E::SignalType::Bus:
				{
					BusShared childBus = bus->busses().getBus(busSignal.busTypeID);

					if (childBus == nullptr)
					{
						result = false;
						continue;
					}

					busChildSignal = createBusParentSignal(ualItem, sChild, childBus, QUuid(), busSignal.caption, lm);
				}
				break;

			default:
				assert(false);
			}

			if (busChildSignal != nullptr)
			{
				result &= busParentSignal->appendBusChildSignal(busSignal.signalID, busChildSignal);
			}
		}

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete busParentSignal;
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		if (autoSignalPtr != nullptr)
		{
			m_compiler.signalSet()->append(autoSignalPtr);
		}

		return busParentSignal;
	}

	bool UalSignalsMap::appendRefPin(const UalItem* ualItem, QUuid pinUuid, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		ualSignal->setUalItem(ualItem);

		if (QHash<UalSignal*, UalSignal*>::contains(ualSignal) == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// ualSignal must exists!
			return false;
		}

		UalSignal* existsSignal = m_pinToSignalMap.value(pinUuid, nullptr);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				return true;
			}

			assert(false);
			LOG_INTERNAL_ERROR(m_log);				// link to this pin is already exists
			return false;
		}

		appendPinRefToSignal(pinUuid, ualSignal);

		return true;
	}

	bool UalSignalsMap::appendRefSignal(Signal* s, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr || s == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (QHash<UalSignal*, UalSignal*>::contains(ualSignal) == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// ualSignal must exists!
			return false;
		}

		UalSignal* existsSignal = m_ptrToSignalMap.value(s, nullptr);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				// ref to same signal, its Ok
				//
				return true;
			}

//			LOG_INTERNAL_ERROR(m_log);			// ref of same appSignalID to different UalSignals, WTF?

			QString msg = QString("Signal %1 try appendRef to %2 and %3 ual signals").
					arg(s->appSignalID()).arg(ualSignal->refSignalIDsJoined()).arg(existsSignal->refSignalIDsJoined());

			LOG_INTERNAL_ERROR_MSG(m_log, msg);

			return false;
		}

		existsSignal = m_idToSignalMap.value(s->appSignalID(), nullptr);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				// ref to same signal, its Ok
				//
				return true;
			}

			LOG_INTERNAL_ERROR(m_log);			// ref of same appSignalID to different UalSignals, WTF?
			return false;
		}

		bool result = ualSignal->appendRefSignal(s, false);

		if (result == false)
		{
			return false;
		}

		m_idToSignalMap.insert(s->appSignalID(), ualSignal);

		if (ualSignal->isBus() == false)
		{
			return true;
		}

		BusShared bus = ualSignal->bus();

		if (bus == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		for(const BusSignal& busSignal : bus->busSignals())
		{
			Signal* newSignal = m_compiler.signalSet()->appendBusChildSignal(*s, bus, busSignal);

			result &= ualSignal->appendBusChildRefSignals(busSignal.signalID, newSignal);
		}

		return result;
	}

	void UalSignalsMap::clear()
	{
		for(UalSignal* ualSignal : (*this))
		{
			if (ualSignal == nullptr)
			{
				assert(false);
				continue;
			}

			delete ualSignal;
		}

		m_idToSignalMap.clear();
		m_pinToSignalMap.clear();
		m_signalToPinsMap.clear();
	}

	bool UalSignalsMap::getReport(QStringList& report) const
	{
		QStringList signalIDs;

		for(UalSignal* ualSignal : (*this))
		{
			if (ualSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (ualSignal->isBusChild() == true)
			{
				continue;
			}

			signalIDs.append(ualSignal->appSignalID());
		}

		signalIDs.sort();

		for(const QString& signalID : signalIDs)
		{
			UalSignal* ualSignal = m_idToSignalMap.value(signalID, nullptr);

			if (ualSignal == nullptr)
			{
				assert(false);
				continue;
			}

			QString str;

			if (ualSignal->isConst())
			{
				str.append("const;");
			}
			else
			{
				str.append("var;");
			}

			str.append(E::valueToString<E::SignalType>(ualSignal->signalType()));
			str += ";";

			str.append(E::valueToString<E::SignalInOutType>(ualSignal->inOutType()));
			str += ";";

			str.append(E::valueToString<E::AnalogAppSignalFormat>(ualSignal->analogSignalFormat()));
			str += ";";

			str.append(ualSignal->busTypeID());
			str += ";";

			str.append(ualSignal->isAcquired() == true ? "true" : "false");
			str += ";";

			str.append(ualSignal->isBusChild() == true ? "true" : "false");
			str += ";";

			str.append(ualSignal->isTunable() == true ? "true" : "false");
			str += ";";

			str.append(ualSignal->isOptoSignal() == true ? "true" : "false");
			str += ";";

			str.append(QString::number(ualSignal->ualAddr().offset()));
			str += ";";
			str.append(QString::number(ualSignal->ualAddr().bit()));
			str += ";";

			str.append(QString::number(ualSignal->ioBufAddr().offset()));
			str += ";";
			str.append(QString::number(ualSignal->ioBufAddr().bit()));
			str += ";";

			str.append(QString::number(ualSignal->regBufAddr().offset()));
			str += ";";
			str.append(QString::number(ualSignal->regBufAddr().bit()));
			str += ";";

			QStringList refSignalIDs;

			ualSignal->refSignalIDs(&refSignalIDs);

			str.append(QString::number(refSignalIDs.count()));
			str += ";";

			str.append(refSignalIDs.join(";"));
			str += ";";

			QList<QUuid> pinsRef = m_signalToPinsMap.values(ualSignal);

			// if pinsRef.count() == 0 - is not an error
			// for example, all acquired input discretes appends to ualSignals even if don't used in UAL

			str += QString::number(pinsRef.count());

			report.append(str);
		}

		return true;
	}

	void UalSignalsMap::initDiscreteSignalsHeap(int startAddrW, int sizeW)
	{
		m_discreteSignalsHeap.init(startAddrW, sizeW);
	}

	int UalSignalsMap::getDiscreteSignalsHeapSizeW() const
	{
		return m_discreteSignalsHeap.getHeapUsedSizeW();
	}

	void UalSignalsMap::initAnalogAndBusSignalsHeap(int startAddrW, int sizeW)
	{
		m_analogAndBusSignalsHeap.init(startAddrW, sizeW);
	}

	int UalSignalsMap::getAnalogAndBusSignalsHeapSizeW() const
	{
		return m_analogAndBusSignalsHeap.getHeapUsedSizeW();
	}

	Address16 UalSignalsMap::getSignalWriteAddress(const UalSignal& ualSignal)
	{
		if (ualSignal.isHeapPlaced() == false)
		{
			return ualSignal.ualAddr();
		}

		Q_ASSERT(ualSignal.ualAddr().isValid() == false);		// for heap placed signals ualAddr should not be set

		if (ualSignal.isDiscrete() == true)
		{
			return m_discreteSignalsHeap.getAddressForWrite(ualSignal);
		}

		return m_analogAndBusSignalsHeap.getAddressForWrite(ualSignal);
	}

	Address16 UalSignalsMap::getSignalReadAddress(const UalSignal& ualSignal, bool decrementReadCount)
	{
		if (ualSignal.isHeapPlaced() == false)
		{
			return ualSignal.ualAddr();
		}

		Q_ASSERT(ualSignal.ualAddr().isValid() == false);		// for heap placed signals ualAddr should not be set

		if (ualSignal.isDiscrete() == true)
		{
			return m_discreteSignalsHeap.getAddressForRead(ualSignal, decrementReadCount);
		}

		return m_analogAndBusSignalsHeap.getAddressForRead(ualSignal, decrementReadCount);
	}

	void UalSignalsMap::removeSignalFromHeap(const UalSignal& ualSignal)
	{
		if (ualSignal.isHeapPlaced() == false)
		{
			return;
		}

		switch(ualSignal.signalType())
		{
		case E::SignalType::Discrete:
			m_discreteSignalsHeap.removeItem(ualSignal);
			break;

		case E::SignalType::Analog:
		case E::SignalType::Bus:
			m_analogAndBusSignalsHeap.removeItem(ualSignal);
			break;

		default:
			Q_ASSERT(false);
		}
	}

	void UalSignalsMap::finalizeHeaps()
	{
		m_discreteSignalsHeap.finalize();
		m_analogAndBusSignalsHeap.finalize();
	}

	void UalSignalsMap::getHeapsLog(QStringList* log) const
	{
		TEST_PTR_RETURN(log);

		log->append(QString("Discrete signals heap log:"));
		log->append(QString());
		log->append(m_discreteSignalsHeap.getHeapLog());
		log->append(QString());
		log->append(QString().fill('=', 120));
		log->append(QString());
		log->append(QString("Analog and Bus signals heap log:"));
		log->append(QString());
		log->append(m_analogAndBusSignalsHeap.getHeapLog());
	}

	UalSignal* UalSignalsMap::privateCreateAutoSignal(const UalItem* ualItem,
											   QUuid outPinUuid,
											   E::SignalType signalType,
											   E::AnalogAppSignalFormat analogFormat,
											   std::optional<int> expectedReadCount)
	{
		TEST_PTR_LOG_RETURN_NULLPTR(ualItem, m_log);

		const LogicPin* outPin = ualItem->getPin(outPinUuid);

		TEST_PTR_LOG_RETURN_NULLPTR(outPin, m_log);

		QString signalID = QString("%1_%2_%3_%4").
								arg(UalSignal::AUTO_SIGNAL_ID_PREFIX).
								arg(m_compiler.lmEquipmentID()).
								arg(ualItem->label()).
								arg(outPin->caption());

		signalID = signalID.toUpper().remove(QRegularExpression("[^#A-Z0-9_]"));

		UalSignal* ualSignal = m_idToSignalMap.value(signalID, nullptr);

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			assert(false);
			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		// create new auto signal
		//
		ualSignal = new UalSignal;

		Signal* autoSignalPtr = nullptr;

		bool result = ualSignal->createAutoSignal(ualItem, signalID, signalType, analogFormat, &autoSignalPtr);

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete ualSignal;
			return nullptr;
		}

		result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			DELETE_IF_NOT_NULL(autoSignalPtr);
			delete ualSignal;
			return nullptr;
		}

		if (autoSignalPtr != nullptr)
		{
			m_compiler.signalSet()->append(autoSignalPtr);
		}
		else
		{
			assert(false);
		}

		// signals heap support
		//
		if (expectedReadCount > 0)
		{
			ualSignal->setHeapPlaced();

			switch(ualSignal->signalType())
			{
			case E::SignalType::Discrete:
				m_discreteSignalsHeap.appendItem(*ualSignal, expectedReadCount);
				break;

			case E::SignalType::Analog:
			case E::SignalType::Bus:
				m_analogAndBusSignalsHeap.appendItem(*ualSignal, expectedReadCount);
				break;

			default:

				Q_ASSERT(false);
			}
		}
		//
		// signals heap support

		return ualSignal;
	}

	bool UalSignalsMap::insertNew(QUuid pinUuid, UalSignal* newUalSignal)
	{
		if (newUalSignal == nullptr || newUalSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (QHash<UalSignal*, UalSignal*>::contains(newUalSignal) == true)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Signal* s = newUalSignal->signal();

		QString signalID = s->appSignalID();

		if (m_idToSignalMap.contains(signalID) == true)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (pinUuid.isNull() == false && m_pinToSignalMap.contains(pinUuid) == true)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (m_ptrToSignalMap.contains(s) == true)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		// all right - insert signal in maps

		insert(newUalSignal, newUalSignal);

		m_idToSignalMap.insert(signalID, newUalSignal);

		m_ptrToSignalMap.insert(s, newUalSignal);

		if (pinUuid.isNull() == false)
		{
			appendPinRefToSignal(pinUuid, newUalSignal);
		}

		return true;
	}

	void UalSignalsMap::appendPinRefToSignal(QUuid pinUuid, UalSignal* ualSignal)
	{
		if (pinUuid.isNull() == true)
		{
			return;							// is not an error
		}

		if (ualSignal == nullptr)
		{
			assert(false);
			return;
		}

		m_pinToSignalMap.insert(pinUuid, ualSignal);
		m_signalToPinsMap.insertMulti(ualSignal, pinUuid);
	}

	QString UalSignalsMap::getAutoSignalID(const UalItem* appItem, const LogicPin& outputPin)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return "";
		}

		QString strID = QString("#AUTO_%1_%2").arg(appItem->label()).arg(outputPin.caption());

		strID = strID.toUpper().remove(QRegularExpression("[^#A-Z0-9_]"));

		return strID;
	}

	bool UalSignalsMap::getAnalogFormat(const LogicAfbSignal& afbSignal, E::AnalogAppSignalFormat* analogFormat)
	{
		if (analogFormat == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		switch(afbSignal.type())
		{
		case E::SignalType::Analog:

			if (afbSignal.dataFormat() == E::DataFormat::Float && afbSignal.size() == SIZE_32BIT)
			{
				*analogFormat = E::AnalogAppSignalFormat::Float32;
				return true;
			}

			if (afbSignal.dataFormat() == E::DataFormat::SignedInt && afbSignal.size() == SIZE_32BIT)
			{
				*analogFormat = E::AnalogAppSignalFormat::SignedInt32;
				return true;
			}

			return false;

		case E::SignalType::Discrete:
		case E::SignalType::Bus:

			return true;

		default:
			;
		}

		return false;
	}
}
