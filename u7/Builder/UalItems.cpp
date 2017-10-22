#include "UalItems.h"

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
		}

		return hasBusInputs;
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

	bool AfblsMap::addInstance(UalAfb* ualAfb)
	{
		if (ualAfb == nullptr)
		{
			assert(false);
			return false;
		}

		QString afbStrID = ualAfb->strID();

		if (contains(afbStrID) == false)
		{
			assert(false);			// unknown FBL strID
			return false;
		}

		Afbl* afbl = (*this)[afbStrID];

		if (afbl == nullptr)
		{
			assert(false);
			return 0;
		}

		int instance = 0;

		QString instantiatorID = ualAfb->instantiatorID();

		if (afbl->hasRam())
		{
			int opCode = afbl->opCode();

			if (m_fblInstance.contains(opCode))
			{
				instance = m_fblInstance[opCode];

				instance++;

				m_fblInstance[opCode] = instance;

				m_nonRamFblInstance.insert(instantiatorID, instance);
			}
			else
			{
				assert(false);		// unknown opcode
			}
		}
		else
		{
			// Calculate non-RAM Fbl instance
			//
			if (m_nonRamFblInstance.contains(instantiatorID))
			{
				instance = m_nonRamFblInstance.value(instantiatorID);
			}
			else
			{
				int opCode = afbl->opCode();
				if (m_fblInstance.contains(opCode))
				{
					instance = m_fblInstance[opCode];

					instance++;

					m_fblInstance[opCode] = instance;

					m_nonRamFblInstance.insert(instantiatorID, instance);
				}
				else
				{
					assert(false);		// unknown opcode
				}
			}
		}

		if (instance == 0)
		{
			assert(false);				// invalid instance number
			return false;
		}

		if (instance > MAX_FB_INSTANCE)
		{
			assert(false);				// reached the max instance number
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
		if (!m_fblInstance.contains(logicAfb->opCode()))
		{
			m_fblInstance.insert(logicAfb->opCode(), 0);
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

	// ---------------------------------------------------------------------------------------
	//
	// AppItem class implementation
	//
	// ---------------------------------------------------------------------------------------

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
		m_appLogicItem.m_afbElement = *afbElement.get();
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
		if (m_appLogicItem.m_fblItem->isSignalElement())
		{
			VFrame30::SchemaItemSignal* itemSignal= m_appLogicItem.m_fblItem->toSignalElement();

			if (itemSignal == nullptr)
			{
				assert(false);
				return "";
			}

			return itemSignal->appSignalIds();
		}

		if (m_appLogicItem.m_fblItem->isAfbElement())
		{
			VFrame30::SchemaItemAfb* itemFb= m_appLogicItem.m_fblItem->toAfbElement();

			if (itemFb == nullptr)
			{
				assert(false);
				return "";
			}

			return itemFb->afbStrID();
		}

		if (m_appLogicItem.m_fblItem->isConstElement())
		{
			VFrame30::SchemaItemConst* itemConst= m_appLogicItem.m_fblItem->toSchemaItemConst();

			if (itemConst == nullptr)
			{
				assert(false);
				return "";
			}

			return QString("Const(%1)").arg(itemConst->valueToString());
		}

		assert(false);		// unknown type of item
		return "";
	}

	UalItem::Type UalItem::type() const
	{
		if (m_type != UalItem::Type::Unknown)
		{
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isSignalElement() == true)
		{
			m_type = Type::Signal;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isAfbElement() == true)
		{
			m_type = Type::Afb;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isConstElement() == true)
		{
			m_type = Type::Const;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isTransmitterElement() == true)
		{
			m_type = Type::Transmitter;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isReceiverElement() == true)
		{
			m_type = Type::Receiver;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isTerminatorElement() == true)
		{
			m_type = Type::Terminator;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isBusComposerElement() == true)
		{
			m_type = Type::BusComposer;
			return m_type;
		}

		if (m_appLogicItem.m_fblItem->isBusExtractorElement() == true)
		{
			m_type = Type::BusExtractor;
			return m_type;
		}

		assert(false);

		m_type = Type::Unknown;

		return m_type;
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

	double AppFbParamValue::floatValue() const
	{
		assert(isFloat32() == true);

		return m_floatValue;
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

	UalAfb::UalAfb(const UalItem& appItem) :
		UalItem(appItem)
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

	QString UalAfb::instantiatorID()
	{
		if (m_instantiatorID.isEmpty() == false)
		{
			return m_instantiatorID;
		}

		m_instantiatorID = afb().strID();

		// append instantiator param's values to instantiatorID
		//
		for(const AppFbParamValue& paramValue : m_paramValuesArray)
		{
			if (paramValue.instantiator() == false)
			{
				continue;
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
			if (m_paramValuesArray.contains(opName) == false)
			{
				if (displayError == true)
				{
					// Required parameter '%1' of AFB '%2' is missing.
					//
					m_log->errALC5045(opName, caption(), guid());
				}

				result = false;
			}
		}

		return result;
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
	// AppSignal class implementation
	//
	// ---------------------------------------------------------------------------------------

	UalSignal::UalSignal(Signal* s)
	{
		// regular UalSignal creation

		if (s == nullptr)
		{
			assert(false);
			return;
		}

		m_autoSignalPtr = nullptr;

		appendRefSignal(s, false);

		// input and tuning signals have already been computed
		//
		if (isSource() == true)
		{
			setComputed();
		}
	}

	UalSignal::UalSignal(const QString& constSignalID,
			  E::SignalType constSignalType,
			  E::AnalogAppSignalFormat constAnalogFormat,
			  int constIntValue,
			  float constFloatValue)
	{
		// const UalSignal creation

		m_autoSignalPtr = new Signal;

		m_autoSignalPtr->setAppSignalID(constSignalID);
		m_autoSignalPtr->setCustomAppSignalID(QString(constSignalID).remove("#"));
		m_autoSignalPtr->setCaption(m_autoSignalPtr->customAppSignalID());

		m_autoSignalPtr->setSignalType(constSignalType);
		m_autoSignalPtr->setInOutType(E::SignalInOutType::Internal);
		m_autoSignalPtr->setAnalogSignalFormat(constAnalogFormat);

		switch(constSignalType)
		{
		case E::SignalType::Discrete:
			m_autoSignalPtr->setDataSize(SIZE_1BIT);
			break;

		case E::SignalType::Analog:
			assert(constAnalogFormat == E::AnalogAppSignalFormat::Float32 || constAnalogFormat == E::AnalogAppSignalFormat::SignedInt32);
			m_autoSignalPtr->setDataSize(SIZE_32BIT);
			break;

		default:
			assert(false);
		}

		m_autoSignalPtr->setAcquire(false);

		appendRefSignal(m_autoSignalPtr, false);

		// set Const signal fields

		m_isConst = true;
		m_constIntValue = constIntValue;
		m_constFloatValue = constFloatValue;

		setComputed();
	}

	UalSignal::UalSignal(const QString& signalID,
			  E::SignalType signalType,
			  E::AnalogAppSignalFormat analogFormat)
	{
		// analog or discrete auto UalSignal creation

		m_autoSignalPtr = new Signal;

		m_autoSignalPtr->setAppSignalID(signalID);
		m_autoSignalPtr->setCustomAppSignalID(QString(signalID).remove("#"));
		m_autoSignalPtr->setCaption(m_autoSignalPtr->customAppSignalID());

		m_autoSignalPtr->setSignalType(signalType);
		m_autoSignalPtr->setInOutType(E::SignalInOutType::Internal);
		m_autoSignalPtr->setAnalogSignalFormat(analogFormat);

		switch(signalType)
		{
		case E::SignalType::Discrete:
			m_autoSignalPtr->setDataSize(SIZE_1BIT);
			break;

		case E::SignalType::Analog:
			assert(analogFormat == E::AnalogAppSignalFormat::Float32 || analogFormat == E::AnalogAppSignalFormat::SignedInt32);
			m_autoSignalPtr->setDataSize(SIZE_32BIT);
			break;

		default:
			assert(false);
		}

		m_autoSignalPtr->setAcquire(false);

		appendRefSignal(m_autoSignalPtr, false);
	}

	UalSignal::UalSignal(Signal* s, const QString& lmEquipmentID)
	{
		// Opto UalSignal creation

		if (s == nullptr)
		{
			assert(false);
			return;
		}

		assert(s->equipmentID() != lmEquipmentID);				// s - is a signal from another LM received by Opto connection

		// create new instance of Signal

		m_autoSignalPtr = new Signal(*s);

		m_autoSignalPtr->setEquipmentID(lmEquipmentID);			// associate new signal with current lm
		m_autoSignalPtr->setAcquire(false);

		appendRefSignal(m_autoSignalPtr, true);

		setComputed();
	}


	UalSignal::UalSignal(const QUuid& guid, E::SignalType signalType,
						 E::AnalogAppSignalFormat dataFormat,
						 int dataSize,
						 const UalItem *appItem,
						 const QString& strID)
	{
/*		m_isAutoSignal = true;

		m_signal = new Signal;

		// construct auto AppSignal based on OutputPin
		//
		m_signal->setAppSignalID(strID);
		m_signal->setSignalType(signalType);
		m_signal->setAnalogSignalFormat(dataFormat);
		m_signal->setDataSize(dataSize);

		if (signalType == E::SignalType::Analog && dataSize != SIZE_32BIT)
		{
			assert(false);
		}

		m_signal->setInOutType(E::SignalInOutType::Internal);
		m_signal->setAcquire(false);								// non-registered signal !*/
	}

	UalSignal::UalSignal(const QUuid& guid, const QString& signalID, const QString& busTypeID, int busSizeW)
	{
/*		m_isAutoSignal = true;

		m_signal = new Signal;

		// construct auto AppSignal based on OutputPin
		//
		m_signal->setAppSignalID(signalID);
		m_signal->setSignalType(E::SignalType::Bus);
		m_signal->setBusTypeID(busTypeID);
		m_signal->setDataSize(busSizeW * SIZE_16BIT);
		m_signal->setInOutType(E::SignalInOutType::Internal);
		m_signal->setAcquire(false);								// non-registered signal !*/
	}

	UalSignal::~UalSignal()
	{
		if (m_autoSignalPtr != nullptr)
		{
			delete m_autoSignalPtr;
		}

		m_refSignals.clear();
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

		// In UalSignal, Input, Tuningable and Opto signals treat as Source

		m_isInput |= s->isInput();
		m_isTuningable |= s->enableTuning();
		m_isOptoSignal |= isOptoSignal;

		// UalSignal can be Input and Output simultaneously (also as Tuningable and Output, OptoSignal and Output)
		// for example, if Input Signal directly connected to Output Signal (or Tuningable => Output, OptoSignal => Output)
		// in this case m_ualAddress set to Sourcet signal ioBufAddr and memory for that signal is not allocate (used ioBuf memory)
		// value of Source signal can't be changed by UAL

		m_isOutput |= s->isOutput();

		m_isAcquired |= s->isAcquired();

		return true;
	}

	void UalSignal::appSignalIDs(QStringList& appSignalIDs)
	{
		appSignalIDs.clear();

		for(Signal* s : m_refSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			appSignalIDs.append(s->appSignalID());
		}
	}

	Signal* UalSignal::firstSignal() const
	{
		if (m_refSignals.count() < 1)
		{
			assert(false);
			return nullptr;
		}

		return m_refSignals[0];
	}

	bool UalSignal::isCompatible(const Signal* s) const
	{
		if (s == nullptr)
		{
			assert(false);
			return false;
		}

		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			assert(false);
			return false;
		}

		return m_refSignals[0]->isCompatibleFormat(*s);
	}

	bool UalSignal::isCompatible(const LogicAfbSignal& afbSignal) const
	{
		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			assert(false);
			return false;
		}

		if (afbSignal.isBus() == true)
		{
			assert(false);			// should be impelmented
			return false;
//			m_signals[0]->isCompatibleFormat(E::SignalType::Bus);
		}
		return m_refSignals[0]->isCompatibleFormat(afbSignal.type(), afbSignal.dataFormat(), afbSignal.size(), afbSignal.byteOrder());
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

		return m_constIntValue == 0 ? 0 : 1;
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

		return m_constFloatValue;
	}

	bool UalSignal::setUalAddr(Address16 ualAddr)
	{
		if (m_isConst == true)
		{
			assert(false);					// for Const signals ualAddr isn't assigned
			return false;
		}

		assert(ualAddr.isValid() == true);

		if (m_ualAddr.isValid() == true)
		{
			assert(false);				// m_ualAddr is already set
			return false;
		}

		m_ualAddr = ualAddr;

		// set same ual address for all associated signals

		for(Signal* s : m_refSignals)
		{
			assert(s->ualAddr().isValid() == false);

			s->setUalAddr(ualAddr);
		}

		return true;
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
		// sorting m_refSignals by m_refSignals[i]->appSignalID ascending
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

	Signal* UalSignal::getTuningableSignal()
	{
		Signal* tuningableSignal = nullptr;

		for(Signal* s : m_refSignals)
		{
			if (s->enableTuning() == true)
			{
				tuningableSignal = s;
				break;
			}
		}

		return tuningableSignal;
	}


	QStringList UalSignal::refSignalsIDs()
	{
		QStringList list;

		for(Signal* s : m_refSignals)
		{
			list.append(s->appSignalID());
		}

		return list;
	}

	QStringList UalSignal::acquiredRefSignalsIDs()
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



	// ---------------------------------------------------------------------------------------
	//
	// AppSignalsMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	UalSignalsMap::UalSignalsMap(ModuleLogicCompiler& compiler, IssueLogger* log) :
		m_compiler(compiler),
		m_log(log)
	{
	}


	UalSignalsMap::~UalSignalsMap()
	{
		clear();
	}

	UalSignal* UalSignalsMap::createSignal(Signal* s)
	{
		return createSignal(s, QUuid());
	}

	UalSignal* UalSignalsMap::createSignal(Signal* s, QUuid outPinUuid)
	{
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
			assert(ualSignal->signal() == s);
			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		// create new signal
		//
		ualSignal = new UalSignal(s);

		bool result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			delete ualSignal;
			return false;
		}

		return ualSignal;
	}

	UalSignal* UalSignalsMap::createConstSignal(E::SignalType constSignalType,
												E::AnalogAppSignalFormat constAnalogFormat,
												const UalConst* ualConst,
												QUuid outPinUuid)
	{
		QString constSignalID = QString("#AUTO_CONST_%1").arg(ualConst->label());

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

		// create new const signal
		//
		ualSignal = new UalSignal(constSignalID, constSignalType, constAnalogFormat, ualConst->intValue(), ualConst->floatValue());

		bool result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			delete ualSignal;
			return false;
		}

		return ualSignal;
	}

	UalSignal* UalSignalsMap::createAutoSignal(const UalItem *ualItem, QUuid outPinUuid, const LogicAfbSignal& outAfbSignal)
	{
		QString signalID = QString("#AUTO_SIGNAL_%1_%2").arg(ualItem->label().arg(outAfbSignal.caption()));

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

		E::AnalogAppSignalFormat analogFormat = E::AnalogAppSignalFormat::SignedInt32;

		bool result = getAnalogFormat(outAfbSignal, &analogFormat);

		if (result == false)
		{
			m_log->addItemsIssues(OutputMessageLevel::Error, ualItem->guid(), ualItem->schemaID());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,
							   QString(tr("Invalid AFB's output format %1.%2 (Logic schema %3)")).
							   arg(ualItem->caption()).arg(outAfbSignal.caption()).arg(ualItem->schemaID()));
			return false;
		}

		// create new auto signal
		//
		ualSignal = new UalSignal(signalID, outAfbSignal.type(), analogFormat);

		result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			delete ualSignal;
			return false;
		}

		return ualSignal;
	}

	UalSignal* UalSignalsMap::createOptoSignal(const UalItem* ualItem, QUuid outPinUuid)
	{
		if (ualItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		const UalReceiver* receiver = ualItem->ualReceiver();

		QString signalID = receiver->appSignalId();

		UalSignal* ualSignal = m_idToSignalMap.value(signalID, nullptr);

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		Signal* s = m_compiler.signalSet().getSignal(signalID);

		if (s == nullptr)
		{
			m_log->errALC5000(signalID, ualItem->guid(), ualItem->schemaID());
			return nullptr;
		}

		// create opto signal
		//
		ualSignal = new UalSignal(s, m_compiler.lmEquipmentID());

		bool result = insertNew(outPinUuid, ualSignal);

		if (result == false)
		{
			delete ualSignal;
			return false;
		}

		return ualSignal;
	}

	bool UalSignalsMap::appendRefPin(QUuid pinUuid, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
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

		UalSignal* existsSignal = m_pinToSignalMap.value(pinUuid, nullptr);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				assert(false);						// is not an error, but why?
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

			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// ref of same appSignalID to different UalSignals, WTF?
			return false;
		}

		existsSignal = m_idToSignalMap.value(s->appSignalID(), nullptr);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				// ref to same signal, its Ok
				//
				assert(false);					// for debug
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

		return true;
	}

	bool UalSignalsMap::insertUalSignal(const UalItem* ualSignal)
	{
/*		if (ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (ualSignal->isSignal() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QString appSignalID = ualSignal->strID();

		if (appSignalID[0] != '#')
		{
			appSignalID = "#" + appSignalID;
		}

		Signal* s = m_compiler.getSignal(appSignalID);

		if (s == nullptr)
		{
			// Signal identifier '%1' is not found.
			//
			m_compiler.log()->errALC5000(appSignalID, ualSignal->guid());
			return false;
		}

		UalSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(appSignalID) == true)
		{
			appSignal = m_signalStrIdMap[appSignalID];
		}
		else
		{
			appSignal = new UalSignal(s, ualSignal);

			m_signalStrIdMap.insert(appSignalID, appSignal);
		}

		assert(appSignal != nullptr);

		HashedVector<QUuid, UalSignal*>::insert(ualSignal->guid(), appSignal);

		// qDebug() << "Insert signal" << ualSignal->guid().toString() << appSignalID;
*/
		return true;
	}

	bool UalSignalsMap::insertNonBusAutoSignal(const UalAfb* appFb, const LogicPin& outputPin)
	{
/*		if (appFb == nullptr )
		{
			LOG_NULLPTR_ERROR(m_log);
		}

		// insert "auto" signal bound to AFB output pin
		//
		const LogicAfbSignal afbSignal = m_compiler.getAfbSignal(appFb->afb().strID(), outputPin.afbOperandIndex());

		QUuid outPinGuid = outputPin.guid();

		QString autoSignalID = getAutoSignalID(appFb, outputPin);

		E::AnalogAppSignalFormat analogSignalFormat;
		int dataSize = 1;

		switch(afbSignal.type())
		{
		case E::SignalType::Analog:
			{
				switch(afbSignal.dataFormat())		// Afb::AfbDataFormat
				{
				case E::DataFormat::Float:
					analogSignalFormat = E::AnalogAppSignalFormat::Float32;
					dataSize = FLOAT32_SIZE;
					break;

				case E::DataFormat::SignedInt:
					analogSignalFormat = E::AnalogAppSignalFormat::SignedInt32;
					dataSize = SIGNED_INT32_SIZE;
					break;

				case E::DataFormat::UnsignedInt:
					// Uncompatible data format of analog AFB Signal '%1.%2'
					//
					m_log->errALC5057(appFb->afb().caption(), afbSignal.caption(), appFb->guid());
					return false;

				default:
					assert(false);
					return false;
				}
			}
			break;

		case E::SignalType::Discrete:
			dataSize = 1;
			break;

		case E::SignalType::Bus:
			LOG_INTERNAL_ERROR(m_log);
			return false;

		default:
			assert(false);
		}

		UalSignal* appSignal = m_signalStrIdMap.value(autoSignalID, nullptr);

		if (appSignal != nullptr)
		{
			assert(false);							// duplicate StrID
		}
		else
		{
			appSignal = new UalSignal(outPinGuid, afbSignal.type(), analogSignalFormat, dataSize, appFb, autoSignalID);

			// auto-signals always connected to output pin, therefore considered computed
			//
			appSignal->setComputed();

			m_signalStrIdMap.insert(autoSignalID, appSignal);
		}

		HashedVector<QUuid, UalSignal*>::insert(outPinGuid, appSignal);
*/
		return true;
	}

	bool UalSignalsMap::insertBusAutoSignal(const UalItem* appItem, const LogicPin& outputPin, BusShared bus)
	{
/*		if (appItem == nullptr || bus == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		QUuid outPinGuid = outputPin.guid();

		QString autoSignalID = getAutoSignalID(appItem, outputPin);

		UalSignal* appSignal = m_signalStrIdMap.value(autoSignalID, nullptr);

		if (appSignal == nullptr)
		{
			appSignal = new UalSignal(outPinGuid, autoSignalID, bus->busTypeID(), bus->sizeW());

			// auto-signals always connected to output pin, therefore considered computed
			//
			appSignal->setComputed();

			m_signalStrIdMap.insert(autoSignalID, appSignal);
		}
		else
		{
			assert(false);							// duplicate StrID??
		}

		HashedVector<QUuid, UalSignal*>::insert(outPinGuid, appSignal);
*/
		return true;
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

	bool UalSignalsMap::getReport(QStringList& report)
	{
		QStringList signalIDs;

		for(UalSignal* ualSignal : (*this))
		{
			if (ualSignal == nullptr)
			{
				assert(false);
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

			str.append(QString::number(ualSignal->ualAddr().offset()));
			str += ";";
			str.append(QString::number(ualSignal->ualAddr().bit()));
			str += ";";

			QStringList refSignalsIDs;

			ualSignal->appSignalIDs(refSignalsIDs);

			for(const QString& refSignalID : refSignalsIDs)
			{
				str += refSignalID + ";";
			}

			QList<QUuid> pinsRef = m_signalToPinsMap.values(ualSignal);

			// if pinsRef.count() == 0 - is not an error
			// for example, all acquired input discretes appends to ualSignals even if don't used in UAL

			str += QString::number(pinsRef.count());

			report.append(str);
		}

		return true;
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
			assert(false);
			return false;
		}

		QString signalID = newUalSignal->signal()->appSignalID();

		if (m_idToSignalMap.contains(signalID) == true)
		{
			assert(false);
			return false;
		}

		if (m_pinToSignalMap.contains(pinUuid) == true)
		{
			assert(false);
			return false;
		}

		Signal* firstSignal = newUalSignal->firstSignal();

		if (firstSignal == nullptr)
		{
			assert(false);
			return false;
		}

		if (m_ptrToSignalMap.contains(firstSignal) == true)
		{
			assert(false);
			return false;
		}

		// all right - insert signal in maps

		insert(newUalSignal, newUalSignal);

		m_idToSignalMap.insert(signalID, newUalSignal);

		m_ptrToSignalMap.insert(firstSignal, newUalSignal);

		appendPinRefToSignal(pinUuid, newUalSignal);

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

		if (afbSignal.isAnalog() == false)
		{
			return true;
		}

		if (afbSignal.dataFormat() == E::DataFormat::Float && afbSignal.size() == SIZE_32BIT)
		{
			*analogFormat = E::AnalogAppSignalFormat::Float32;
		}
		else
		{
			if (afbSignal.dataFormat() == E::DataFormat::SignedInt && afbSignal.size() == SIZE_32BIT)
			{
				*analogFormat = E::AnalogAppSignalFormat::SignedInt32;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

}
