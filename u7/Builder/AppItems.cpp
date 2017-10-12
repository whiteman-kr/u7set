#include "AppItems.h"

#include "ModuleLogicCompiler.h"

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// LogicAfb class implementation
	//
	// ---------------------------------------------------------------------------------------

	LogicAfb::LogicAfb(std::shared_ptr<Afb::AfbElement> afb) :
		m_afb(afb)
	{
		if (m_afb == nullptr)
		{
			assert(false);
			return;
		}
	}

	LogicAfb::~LogicAfb()
	{
	}

	bool LogicAfb::isBusProcessingAfb() const
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

	bool LogicAfb::isBusProcessingAfbChecking() const
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

	AfbMap::~AfbMap()
	{
		clear();
	}

	bool AfbMap::addInstance(AppFb* appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return false;
		}

		QString afbStrID = appFb->strID();

		if (!contains(afbStrID))
		{
			assert(false);			// unknown FBL strID
			return false;
		}

		LogicAfb* fbl = (*this)[afbStrID];

		if (fbl == nullptr)
		{
			assert(false);
			return 0;
		}

		int instance = 0;

		QString instantiatorID = appFb->instantiatorID();

		if (fbl->hasRam())
		{
			int opCode = fbl->opCode();

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
				int opCode = fbl->opCode();
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

		appFb->setInstance(instance);

		return true;
	}

	void AfbMap::insert(std::shared_ptr<Afb::AfbElement> logicAfb)
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

		LogicAfb* afb = new LogicAfb(logicAfb);

		HashedVector<QString, LogicAfb*>::insert(afb->strID(), afb);

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
			if (param.operandIndex() == AppFb::FOR_USER_ONLY_PARAM_INDEX)
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

	void AfbMap::clear()
	{
		for(LogicAfb* fbl : *this)
		{
			delete fbl;
		}

		HashedVector<QString, LogicAfb*>::clear();
	}

	const LogicAfbSignal AfbMap::getAfbSignal(const QString& afbStrID, int signalIndex)
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

	AppItem::AppItem()
	{
	}

	AppItem::AppItem(const AppItem& appItem) :
		QObject()
	{
		m_appLogicItem = appItem.m_appLogicItem;
	}

	AppItem::AppItem(const AppLogicItem& appLogicItem) :
		m_appLogicItem(appLogicItem)
	{
	}

	AppItem::AppItem(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg)
	{
		init(afbElement, errorMsg);
	}

	bool AppItem::init(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg)
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

	QString AppItem::strID() const
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

	AppItem::Type AppItem::type() const
	{
		if (isSignal() == true)
		{
			return Type::Signal;
		}

		if (isAfb() == true)
		{
			return Type::Afb;
		}

		if (isConst() == true)
		{
			return Type::Const;
		}

		if (isTransmitter() == true)
		{
			return Type::Transmitter;
		}

		if (isReceiver() == true)
		{
			return Type::Receiver;
		}

		if (isTerminator() == true)
		{
			return Type::Terminator;
		}

		if (isBusComposer() == true)
		{
			return Type::BusComposer;
		}

		if (isBusExtractor() == true)
		{
			return Type::BusExtractor;
		}

		assert(false);

		return Type::Unknown;
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

	AppFb::AppFb(const AppItem& appItem) :
		AppItem(appItem)
	{
		// initialize m_paramValuesArray
		//
		for(const Afb::AfbParam& afbParam : appItem.params())
		{
			AppFbParamValue value(afbParam);

			m_paramValuesArray.insert(afbParam.opName(), value);
		}
	}

	bool AppFb::isConstComaparator() const
	{
		return opcode() == CONST_COMPARATOR_OPCODE;
	}

	bool AppFb::isDynamicComaparator() const
	{
		return opcode() == DYNAMIC_COMPARATOR_OPCODE;
	}

	bool AppFb::isComparator() const
	{
		quint16 oc = opcode();

		return oc ==  CONST_COMPARATOR_OPCODE || oc == DYNAMIC_COMPARATOR_OPCODE;
	}

	QString AppFb::instantiatorID()
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

	bool AppFb::getAfbParamByIndex(int index, LogicAfbParam* afbParam) const
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


	bool AppFb::getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const
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

	bool AppFb::getAfbSignalByPinUuid(QUuid pinUuid, LogicAfbSignal* afbSignal) const
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
				  QString(tr("Not found signal with pin Uuidx = %1 in FB %2")).arg(pinUuid.toString()).arg(caption()));

		return false;
	}


	bool AppFb::checkRequiredParameters(const QStringList& requiredParams)
	{
		return checkRequiredParameters(requiredParams, true);
	}

	bool AppFb::checkRequiredParameters(const QStringList& requiredParams, bool displayError)
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

	bool AppFb::checkUnsignedInt(const AppFbParamValue& paramValue)
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

	bool AppFb::checkUnsignedInt16(const AppFbParamValue& paramValue)
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

	bool AppFb::checkUnsignedInt32(const AppFbParamValue& paramValue)
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

	bool AppFb::checkSignedInt32(const AppFbParamValue& paramValue)
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

	bool AppFb::checkFloat32(const AppFbParamValue& paramValue)
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

	AppFbMap::~AppFbMap()
	{
		clear();
	}

	AppFb* AppFbMap::insert(AppFb *appFb)
	{
		if (appFb == nullptr)
		{
			assert(false);
			return nullptr;
		}

		appFb->setNumber(m_fbNumber);

		m_fbNumber++;

		HashedVector<QUuid, AppFb*>::insert(appFb->guid(), appFb);

		return appFb;
	}


	void AppFbMap::clear()
	{
		for(AppFb* appFb : *this)
		{
			delete appFb;
		}

		HashedVector<QUuid, AppFb*>::clear();
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppSignal class implementation
	//
	// ---------------------------------------------------------------------------------------

	AppSignal::AppSignal(Signal *signal, const AppItem* ualSignal) :
		m_signal(signal),
		m_appItem(ualSignal)
	{
		m_isAutoSignal = false;

		// construct AppSignal based on real signal
		//
		if (m_signal == nullptr)
		{
			assert(false);
			return;
		}

		// believe that all input and tuning signals have already been computed
		//
		if ( m_signal->isInput() == true ||
			(m_signal->isInternal() == true && m_signal->enableTuning() == true))
		{
			setComputed();
		}
	}

	AppSignal::AppSignal(const QUuid& guid, E::SignalType signalType,
						 E::AnalogAppSignalFormat dataFormat,
						 int dataSize,
						 const AppItem *appItem,
						 const QString& strID) :
		m_appItem(appItem),
		m_guid(guid)
	{
		m_isAutoSignal = true;

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
		m_signal->setAcquire(false);								// non-registered signal !
	}

	AppSignal::AppSignal(const QUuid& guid, const QString& signalID, const QString& busTypeID, int busSizeW)
	{
		m_isAutoSignal = true;

		m_signal = new Signal;

		// construct auto AppSignal based on OutputPin
		//
		m_signal->setAppSignalID(signalID);
		m_signal->setSignalType(E::SignalType::Bus);
		m_signal->setBusTypeID(busTypeID);
		m_signal->setDataSize(busSizeW * SIZE_16BIT);
		m_signal->setInOutType(E::SignalInOutType::Internal);
		m_signal->setAcquire(false);								// non-registered signal !
	}

	AppSignal::~AppSignal()
	{
		if (m_isAutoSignal == true)
		{
			delete m_signal;
		}
	}

	const AppItem& AppSignal::appItem() const
	{
		assert(m_appItem != nullptr);

		return *m_appItem;
	}

	QUuid AppSignal::guid() const
	{
		if (m_appItem != nullptr)
		{
			return m_appItem->guid();
		}

		assert(false);

		return QUuid();
	}

	bool AppSignal::isCompatibleDataFormat(const LogicAfbSignal& afbSignal) const
	{
		return m_signal->isCompatibleFormat(afbSignal.type(), afbSignal.dataFormat(), afbSignal.size(), afbSignal.byteOrder());
	}

	QString AppSignal::schemaID() const
	{
		if (m_appItem != nullptr)
		{
			return m_appItem->schemaID();
		}

		assert(false);

		return "";
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppSignalsMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	AppSignalMap::AppSignalMap(ModuleLogicCompiler& compiler, IssueLogger* log) :
		m_compiler(compiler),
		m_log(log)
	{
	}


	AppSignalMap::~AppSignalMap()
	{
		clear();
	}

	bool AppSignalMap::insertUalSignal(const AppItem* ualSignal)
	{
		if (ualSignal == nullptr)
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

		AppSignal* appSignal = nullptr;

		if (m_signalStrIdMap.contains(appSignalID) == true)
		{
			appSignal = m_signalStrIdMap[appSignalID];
		}
		else
		{
			appSignal = new AppSignal(s, ualSignal);

			m_signalStrIdMap.insert(appSignalID, appSignal);
		}

		assert(appSignal != nullptr);

		HashedVector<QUuid, AppSignal*>::insert(ualSignal->guid(), appSignal);

		return true;
	}

	bool AppSignalMap::insertAfbOutputAutoSignal(const AppFb* appFb, const LogicPin& outputPin)
	{
		if (appFb == nullptr )
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
			assert(false);		// insertBusAutoSignal should be called
			dataSize = -1;		// real data size of bus should be assigned !!!
			break;

		default:
			assert(false);
		}

		AppSignal* appSignal = m_signalStrIdMap.value(autoSignalID, nullptr);

		if (appSignal != nullptr)
		{
			assert(false);							// duplicate StrID
		}
		else
		{
			appSignal = new AppSignal(outPinGuid, afbSignal.type(), analogSignalFormat, dataSize, appFb, autoSignalID);

			// auto-signals always connected to output pin, therefore considered computed
			//
			appSignal->setComputed();

			m_signalStrIdMap.insert(autoSignalID, appSignal);
		}

		HashedVector<QUuid, AppSignal*>::insert(outPinGuid, appSignal);

		return true;
	}

	bool AppSignalMap::insertBusAutoSignal(const AppItem* appItem, const LogicPin& outputPin, const QString& busTypeID, int busSizeW)
	{
		if (appItem == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}
		QUuid outPinGuid = outputPin.guid();

		QString autoSignalID = getAutoSignalID(appItem, outputPin);

		AppSignal* appSignal = m_signalStrIdMap.value(autoSignalID, nullptr);

		if (appSignal == nullptr)
		{
			appSignal = new AppSignal(outPinGuid, autoSignalID, busTypeID, busSizeW);

			// auto-signals always connected to output pin, therefore considered computed
			//
			appSignal->setComputed();

			m_signalStrIdMap.insert(autoSignalID, appSignal);
		}
		else
		{
			assert(false);							// duplicate StrID??
		}

		HashedVector<QUuid, AppSignal*>::insert(outPinGuid, appSignal);

		return true;
	}

	AppSignal* AppSignalMap::getSignal(const QString& appSignalID)
	{
		return m_signalStrIdMap.value(appSignalID, nullptr);
	}

	bool AppSignalMap::containsSignal(const QString& appSignalID) const
	{
		return m_signalStrIdMap.contains(appSignalID);
	}

	void AppSignalMap::clear()
	{
		for(AppSignal* appSignal : m_signalStrIdMap)
		{
			delete appSignal;
		}

		m_signalStrIdMap.clear();

		HashedVector<QUuid, AppSignal*>::clear();
	}

	QString AppSignalMap::getAutoSignalID(const AppItem* appItem, const LogicPin& outputPin)
	{
		if (appItem == nullptr)
		{
			assert(false);
			return "";
		}

		QString strID = QString("#%1_%2").arg(appItem->label()).arg(outputPin.caption());

		strID = strID.toUpper().remove(QRegularExpression("[^#A-Z0-9_]"));

		return strID;
	}

}
