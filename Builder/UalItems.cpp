#include "UalItems.h"
#include "ModuleLogicCompiler.h"
#include <VFrame30/Schema.h>

namespace Builder
{
	// ---------------------------------------------------------------------------------------
	//
	// AfbComponents class implementation
	//
	// ---------------------------------------------------------------------------------------

	bool AfbComponents::LogicInfo::isValid() const
	{
		return	opCode != -1 &&
				confIndex != -1 &&
				operandQuantityIndex != -1 &&
				busWidthIndex != -1 &&
				firstInIndex != -1 &&
				resultIndex != -1 &&
				confOr != -1 &&
				confAnd != -1 &&
				minOperandsCount != -1 &&
				maxOperandsCount != -1;
	}

	AfbComponents::ComponentInfo::ComponentInfo(const QString& compCaption,
												int compMaxInstCount,
												bool compHasRam)
	{
		caption = compCaption;
		maxInstCount = compMaxInstCount;
		hasRam = compHasRam;
	}

	AfbComponents::AfbComponents()
	{
	}

	AfbComponents::~AfbComponents()
	{
	}

	void AfbComponents::init(LmDescriptionConstShared lmDescription)
	{
		TEST_PTR_RETURN(lmDescription);

		if (lmDescription->afbComponents().size() == 0 ||
			lmDescription->afbElements().size() == 0)
		{
			return;		// it is Ok, for example BVB15
		}

		for(const auto& [opCode, afbComp] : lmDescription->afbComponents())
		{
			TEST_PTR_CONTINUE(afbComp);

			Q_ASSERT(m_componentsInfo.contains(afbComp->opCode()) == false);

			m_componentsInfo.emplace(afbComp->opCode(),
								   ComponentInfo(afbComp->caption(), afbComp->maxInstCount(), afbComp->hasRam()));

			if (afbComp->caption() == QStringLiteral("LOGIC"))
			{
				m_logicInfo.opCode = afbComp->opCode();
				m_logicInfo.confIndex = afbComp->pinOpIndex(Afb::PARAM_I_CONF);
				m_logicInfo.operandQuantityIndex = afbComp->pinOpIndex(Afb::PARAM_I_OPRD_QUANT);
				m_logicInfo.busWidthIndex = afbComp->pinOpIndex(Afb::PARAM_I_BUS_WIDTH);
				m_logicInfo.firstInIndex = afbComp->pinOpIndex(Afb::PIN_I_1_OPRD);
				m_logicInfo.resultIndex = afbComp->pinOpIndex(Afb::PIN_O_RESULT);
			}
		}

		for(const AfbElementShared& afbElement : lmDescription->afbElements())
		{
			TEST_PTR_CONTINUE(afbElement);

			//

			if (afbElement->caption() == Afb::AFB_OR)
			{
				Afb::AfbParam param = afbElement->paramByOpName(Afb::PARAM_I_CONF);

				if (param.isValid())
				{
					m_logicInfo.confOr = param.afbParamValue().value().toInt();
				}

				param = afbElement->paramByOpName(Afb::PARAM_I_OPRD_QUANT);

				if (param.isValid())
				{
					m_logicInfo.minOperandsCount = param.lowLimit().toInt();
					m_logicInfo.maxOperandsCount = param.highLimit().toInt();
				}
			}

			//

			if (afbElement->caption() == Afb::AFB_AND)
			{
				Afb::AfbParam param = afbElement->paramByOpName(Afb::PARAM_I_CONF);

				if (param.isValid())
				{
					m_logicInfo.confAnd = param.afbParamValue().value().toInt();
				}
			}

			// checking is this bus processing element
			//
			const std::vector<Afb::AfbSignal>& inputSignals = afbElement->inputSignals();

			bool hasBusInputs = false;

			for(const Afb::AfbSignal& afbInSignal : inputSignals)
			{
				if (afbInSignal.type() == E::SignalType::Bus)
				{
					hasBusInputs = true;
					break;
				}
			}

			const std::vector<Afb::AfbSignal>& outputSignals = afbElement->outputSignals();

			bool hasBusOutputs = false;

			for(const Afb::AfbSignal& afbOutSignal : outputSignals)
			{
				if (afbOutSignal.type() == E::SignalType::Bus)
				{
					hasBusOutputs = true;
					break;
				}
			}

			if (hasBusInputs == true && hasBusOutputs == true)
			{
				m_busProcessingAfbElemets.insert(calcHash(afbElement->strID()));
			}
		}

		Q_ASSERT(m_logicInfo.isValid());
	}

	bool AfbComponents::addInstance(UalAfb* ualAfb, IssueLogger* log)
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

		int opCode = ualAfb->opcode();

		auto instanceIt = m_componentsInfo.find(opCode);

		if (instanceIt == m_componentsInfo.end())
		{
			// Unknown AFB type (opCode = %1) (item %2, schema %3).
			//
			log->errALC5129(opCode, ualAfb->guid(), ualAfb->label(), ualAfb->schemaID());
			return false;
		}

		ComponentInfo& ci = instanceIt->second;

		const QString& instantiatorID = ualAfb->instantiatorID();

		if (ci.hasRam == false)
		{
			auto instantiatorIt = ci.nonRamAfbInstantiators.find(instantiatorID);

			if (instantiatorIt != ci.nonRamAfbInstantiators.end())
			{
				// ualAfb has no RAM and its instantiatorID is already exists - compInfo.curInstance would be used
				//
				int existInstance = instantiatorIt->second;

				ualAfb->setInstance(existInstance);
				return true;
			}
		}

		// ualAfb has RAM or
		// ualAfb hasn't RAM and its instantiatorID is not exist - new instance would be created
		//
		if (ci.maxInstCount > 0)
		{
			if (ci.curInstance + 1 >= ci.maxInstCount)
			{
				// Max instances (%1) of AFB component '%2' is used (Logic schema %3, item %4)
				//
				log->errALC5130(ci.maxInstCount, ci.caption, ualAfb->guid(), ualAfb->schemaID(), ualAfb->label());
				return false;
			}

			ci.curInstance++;
		}
		else
		{
			ci.curInstance = 0;
		}

		if (ci.hasRam == false)
		{
			// append new instantiatorID for non-RAM afb instantiators
			//
			ci.nonRamAfbInstantiators.emplace(instantiatorID, ci.curInstance);
		}

		if (ci.curInstance < 0)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(log);
			return false;
		}

		ualAfb->setInstance(ci.curInstance);

		return true;
	}

	bool AfbComponents::addInstance(int afbOpcode, int* newInstance, IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(newInstance);

		auto it = m_componentsInfo.find(afbOpcode);

		if (it == m_componentsInfo.end())
		{
			// Unknown AFB type (opCode = %1) (item %2, schema %3).
			//
			log->errALC5129(afbOpcode, QUuid(), "", "");
			return false;
		}

		ComponentInfo& ci = it->second;

		if (ci.curInstance + 1 >= ci.maxInstCount)
		{
			// Max instances (%1) of AFB component '%2' is used (Logic schema %3, item %4)
			//
			log->errALC5130(ci.maxInstCount, ci.caption, QUuid(), "", "");
			return false;
		}

		ci.curInstance++;

		*newInstance = ci.curInstance;

		return true;
	}

	int AfbComponents::getUsedInstancesCount(int opCode) const
	{
		auto it = m_componentsInfo.find(opCode);

		if (it == m_componentsInfo.end())
		{
			Q_ASSERT(false);
			return 0;
		}

		const ComponentInfo& ci = it->second;

		return ci.curInstance + 1;
	}

	bool AfbComponents::isBusProcessingAfb(const QString& afbElementStrID) const
	{
		return m_busProcessingAfbElemets.contains(calcHash(afbElementStrID));
	}

	const AfbComponents::LogicInfo& AfbComponents::logicInfo() const
	{
		return m_logicInfo;
	}

	// ---------------------------------------------------------------------------------------
	//
	// UalItem class implementation
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
		// m_appLogicItem.m_afbElement = *afbElement.get();

		m_appLogicItem.m_fblItem = std::shared_ptr<VFrame30::FblItemRect>(
					new VFrame30::SchemaItemAfb(SchemaUnit::Display, *afbElement.get(), &errorMsg));

		// copy parameters
		//
		for(Afb::AfbParam& param : afbElement->params())
		{
			m_appLogicItem.m_fblItem->toAfbElement()->setAfbParamByOpName(param.opName(), param.afbParamValue());
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

		return m_appLogicItem.afbElement().caption() == Afb::SET_FLAGS;
	}

	bool UalItem::isSimLockItem() const
	{
		if (isAfb() == false)
		{
			return false;
		}

		return m_appLogicItem.afbElement().caption() == Afb::SIMLOCK;
	}

	bool UalItem::isMismatchItem() const
	{
		if (isAfb() == false)
		{
			return false;
		}

		return m_appLogicItem.afbElement().caption().startsWith(Afb::MISMATCH);
	}

	bool UalItem::isPackedLogic() const
	{
		return m_appLogicItem.afbElement().isPackedLogic();
	}

    bool UalItem::assignFlags(IssueLogger* log) const
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

		Q_UNUSED(log);
//		LOG_INTERNAL_ERROR_MSG(log, QString("Property AssignFlags is not exists in ualItem %1").
//											arg(schemaItemAfb->label()));
		Q_ASSERT(false);

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

	bool UalItem::hasRam() const
	{
		const Afb::AfbElement& afbElement = afb();

		std::optional<bool> hasRam = afbElement.hasRam();

		if (hasRam.has_value() == true)
		{
			return hasRam.value();
		}

		std::shared_ptr<Afb::AfbComponent> afbComp = afbComponent();

		return afbComp->hasRam();
	}

	const SchemaConst* UalItem::schemaConst() const
	{
		const SchemaConst* ptr = m_appLogicItem.m_fblItem->toSchemaItemConst();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaSignal* UalItem::schemaSignal() const
	{
		const SchemaSignal* ptr = m_appLogicItem.m_fblItem->toSignalElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaAfb* UalItem::schemaAfb() const
	{
		const SchemaAfb* ptr = m_appLogicItem.m_fblItem->toAfbElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaTransmitter* UalItem::schemaTransmitter() const
	{
		const SchemaTransmitter*ptr = m_appLogicItem.m_fblItem->toTransmitterElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaReceiver* UalItem::schemaReceiver() const
	{
		const SchemaReceiver* ptr = m_appLogicItem.m_fblItem->toReceiverElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaBusComposer* UalItem::schemaBusComposer() const
	{
		const SchemaBusComposer* ptr = m_appLogicItem.m_fblItem->toBusComposerElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaBusExtractor* UalItem::schemaBusExtractor() const
	{
		const SchemaBusExtractor* ptr =m_appLogicItem.m_fblItem->toBusExtractorElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaLoopbackSource* UalItem::schemaLoopbackSource() const
	{
		const SchemaLoopbackSource* ptr = m_appLogicItem.m_fblItem->toLoopbackSourceElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	const SchemaLoopbackTarget* UalItem::schemaLoopbackTarget() const
	{
		SchemaLoopbackTarget* ptr = m_appLogicItem.m_fblItem->toLoopbackTargetElement();
		Q_ASSERT(ptr != nullptr);
		return ptr;
	}

	QString UalItem::schemaID() const
	{
		if (m_appLogicItem.m_schema != nullptr)
		{
			return m_appLogicItem.m_schema->schemaId();
		}

		return QString("Internal Processing");
	}

	const SchemaPin* UalItem::getPin(QUuid pinUuid) const
	{
		const std::vector<SchemaPin>& inputPins = inputs();

		for(const SchemaPin& inPin : inputPins)
		{
			if (inPin.guid() == pinUuid)
			{
				return &inPin;
			}
		}

		const std::vector<SchemaPin>& outputPins = outputs();

		for(const SchemaPin& outPin : outputPins)
		{
			if (outPin.guid() == pinUuid)
			{
				return &outPin;
			}
		}

		return nullptr;
	}

	const SchemaPin* UalItem::getPin(const QString& pinCaption) const
	{
		const std::vector<SchemaPin>& inputPins = inputs();

		for(const SchemaPin& inPin : inputPins)
		{
			if (inPin.caption() == pinCaption)
			{
				return &inPin;
			}
		}

		const std::vector<SchemaPin>& outputPins = outputs();

		for(const SchemaPin& outPin : outputPins)
		{
			if (outPin.caption() == pinCaption)
			{
				return &outPin;
			}
		}

		return nullptr;
	}

	bool UalItem::setParamValueByCaption(const QString& paramCaption, const QVariant& value)
	{
		std::vector<Afb::AfbParam>& ps = params();

		for(Afb::AfbParam& p : ps)
		{
			if (p.caption() == paramCaption)
			{
				p.afbParamValue().setValue(value);
				return true;
			}
		}

		return false;
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFbParamValue class implementation
	//
	// ---------------------------------------------------------------------------------------

	AfbParamValue::AfbParamValue()
	{
	}

	AfbParamValue::AfbParamValue(const Afb::AfbParam& afbParam)
	{
		Q_ASSERT(afbParam.afbParamValue().reference().isEmpty() == true);

		m_opName = afbParam.opName();
		m_caption = afbParam.caption();
		m_operandIndex = afbParam.operandIndex();
		m_instantiator = afbParam.instantiator();
		m_visible = afbParam.visible();

		m_type = afbParam.type();

		switch(m_type)
		{
		case E::SignalType::Discrete:

			m_dataFormat = E::DataFormat::UnsignedInt;
			m_dataSize = 1;

			break;

		case E::SignalType::Analog:

			m_dataSize = afbParam.size();

			switch(afbParam.dataFormat())
			{
			case E::DataFormat::SignedInt:
				m_dataFormat = E::DataFormat::SignedInt;
				break;

			case E::DataFormat::UnsignedInt:
				m_dataFormat = E::DataFormat::UnsignedInt;
				break;

			case E::DataFormat::Float:
				Q_ASSERT(m_dataSize == SIZE_32BIT);
				m_dataFormat = E::DataFormat::Float;
				break;

			default:
				Q_ASSERT(false);
			}

			break;

		default:
			Q_ASSERT(false);
		}

		setValue(afbParam.afbParamValue().value());
	}

	quint32 AfbParamValue::unsignedIntValue() const
	{
		assert(isUnsignedInt() == true);

		return m_unsignedIntValue;
	}

	void AfbParamValue::setUnsignedIntValue(quint32 value)
	{
		assert(isUnsignedInt() == true);

		m_unsignedIntValue = value;
	}

	qint32 AfbParamValue::signedIntValue() const
	{
		assert(isSignedInt32() == true);

		return m_signedIntValue;
	}

	void AfbParamValue::setSignedIntValue(qint32 value)
	{
		Q_ASSERT(isSignedInt32() == true);

		m_signedIntValue = value;
	}

	float AfbParamValue::floatValue() const
	{
		Q_ASSERT(isFloat32() == true);

		return static_cast<float>(m_floatValue);
	}

	void AfbParamValue::setFloatValue(double value)
	{
		Q_ASSERT(isFloat32() == true);

		m_floatValue = value;
	}

	void AfbParamValue::setValue(const QVariant& qv)
	{
		switch(m_type)
		{
		case E::SignalType::Discrete:
			m_unsignedIntValue = qv.toUInt();
			break;

		case E::SignalType::Analog:

			switch(m_dataFormat)
			{
			case E::DataFormat::SignedInt:
				m_signedIntValue = qv.toInt();
				break;

			case E::DataFormat::UnsignedInt:
				m_unsignedIntValue = qv.toUInt();
				break;

			case E::DataFormat::Float:
				m_floatValue = qv.toFloat();
				break;

			default:
				Q_ASSERT(false);
			}

			break;

		default:
			Q_ASSERT(false);
		}
	}

	QString AfbParamValue::toString() const
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
	// AppFbParamValuesArray class implementation
	//
	// ---------------------------------------------------------------------------------------

	AfbParamValue AfbParamValuesArray::m_nullValue;

	void AfbParamValuesArray::insert(const QString& opName, const AfbParamValue& value)
	{
		size_t index = size();
		push_back(value);
		m_opNameToIndex.insert({opName, index});
	}

	bool AfbParamValuesArray::contains(const QString& opName) const
	{
		return m_opNameToIndex.contains(opName);
	}

	bool AfbParamValuesArray::isEmpty() const
	{
		return empty();
	}

	bool AfbParamValuesArray::hasParamsToInitialization() const
	{
		for(const AfbParamValue& pv : *this)
		{
			if (pv.isNoFbOperand() == false)
			{
				return true;
			}
		}

		return false;
	}

	AfbParamValue& AfbParamValuesArray::operator [] (const QString& opName)
	{
		return const_cast<AfbParamValue&>(find(opName));
	}

	const AfbParamValue& AfbParamValuesArray::operator [] (const QString& opName) const
	{
		return find(opName);
	}

	const AfbParamValue& AfbParamValuesArray::find(const QString& opName) const
	{
		auto it = m_opNameToIndex.find(opName);

		if (it == m_opNameToIndex.end())
		{
			Q_ASSERT(false);
			return m_nullValue;
		}

		return at(it->second);
	}

	// ---------------------------------------------------------------------------------------
	//
	// AppFb class implementation
	//
	// ---------------------------------------------------------------------------------------

	std::set<QString> UalAfb::m_lmsWithLessGreateEqMode;

	UalAfb::UalAfb(const UalItem& appItem, bool isBusProcessingAfb) :
		UalItem(appItem),
		m_isBusProcessing(isBusProcessingAfb)
	{
		// initialize m_paramValuesArray
		//
		for(const Afb::AfbParam& afbParam : appItem.params())
		{
			AfbParamValue value(afbParam);

			m_paramValuesArray.insert(afbParam.opName(), value);
		}

		//

		if (m_lmsWithLessGreateEqMode.empty() == true)
		{
			m_lmsWithLessGreateEqMode.insert(LmDescriptionName::LM1_SR20);
			m_lmsWithLessGreateEqMode.insert(LmDescriptionName::LM1_SR05);
			m_lmsWithLessGreateEqMode.insert(LmDescriptionName::LM8_SR10);
			m_lmsWithLessGreateEqMode.insert(LmDescriptionName::LM11_SR90);
			m_lmsWithLessGreateEqMode.insert(LmDescriptionName::LM_SF41);
		}
	}

	bool UalAfb::isConstComaparator() const
	{
		return opcode() == Afb::CONST_COMPARATOR_OPCODE;
	}

	bool UalAfb::isDynamicComaparator() const
	{
		return opcode() == Afb::DYNAMIC_COMPARATOR_OPCODE;
	}

	bool UalAfb::isComparator() const
	{
		quint16 oc = opcode();

		return	oc ==  Afb::CONST_COMPARATOR_OPCODE ||
				oc == Afb::DYNAMIC_COMPARATOR_OPCODE;
	}

	bool UalAfb::isBusProcessing() const
	{
		return m_isBusProcessing;
	}

	bool UalAfb::isPackedLogic() const
	{
		int oc = opcode();

		return	oc == Afb::PACKED_AND_OPCODE ||
				oc == Afb::PACKED_OR_OPCODE;
	}

	bool UalAfb::isPackedOrLogic() const
	{
		return opcode() == Afb::PACKED_OR_OPCODE;
	}

	bool UalAfb::isPackedAndLogic() const
	{
		return opcode() == Afb::PACKED_AND_OPCODE;
	}

	QString UalAfb::packedLogicID() const
	{
		Q_ASSERT(isPackedLogic());

		return m_appLogicItem.afbElement().packedLogicId();
	}

	int UalAfb::precision() const
	{
		const SchemaAfb* safb = schemaAfb();

		TEST_PTR_RETURN_VALUE(safb, 0);

		return safb->precision();
	}

	const QString& UalAfb::instantiatorID() const
	{
		if (m_instantiatorID.isEmpty() == false)
		{
			return m_instantiatorID;
		}

		m_instantiatorID = QString("opCode:%1").arg(afb().opCode());

		bool firstParam = true;

		// append instantiator param's values to instantiatorID
		//
		for(const AfbParamValue& paramValue : m_paramValuesArray)
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

	bool UalAfb::getAfbParamByIndex(int index, AfbParam* afbParam) const
	{
		const std::vector<AfbParam>& params = afb().params();

		for(const AfbParam& param : params)
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

	const AfbParam* UalAfb::getParamByOpName(const QString& opName) const
	{
		const std::vector<AfbParam>& params = afb().params();

		for(const AfbParam& param : params)
		{
			if (param.opName() == opName)
			{
				return &param;
			}
		}

		return nullptr;
	}

	const AfbParam* UalAfb::getParamByCaption(const QString& caption, bool toLower) const
	{
		const std::vector<AfbParam>& params = afb().params();

		if (toLower)
		{
			QString cpLowercase = caption.toLower();

			for(const AfbParam& param : params)
			{
				if (param.caption().toLower() == cpLowercase)
				{
					return &param;
				}
			}
		}
		else
		{
			for(const AfbParam& param : params)
			{
				if (param.caption() == caption)
				{
					return &param;
				}
			}
		}

		return nullptr;
	}

	int UalAfb::getParamIntValueByOpName(const QString& opName, bool* ok) const
	{
		TEST_PTR_RETURN_VALUE(ok, 0);

		*ok = false;

		const AfbParam* param = getParamByOpName(opName);

		if (param == nullptr)
		{
			return 0;
		}

		int paramValue = param->afbParamValue().value().toInt(ok);

		if (*ok == false)
		{
			Q_ASSERT(false);
			return 0;
		}

		return paramValue;
	}

	bool UalAfb::getAfbSignalByPin(const SchemaPin& pin, AfbSignal* afbSignal) const
	{
		return getAfbSignalByIndex(pin.afbOperandIndex(), afbSignal);
	}

	bool UalAfb::getAfbSignalByIndex(int index, AfbSignal* afbSignal) const
	{
		if (afbSignal == nullptr)
		{
			return false;
		}

		for(const AfbSignal& input : afb().inputSignals())
		{
			if (input.operandIndex() == index)
			{
				*afbSignal = input;
				return true;
			}
		}

		for(const AfbSignal& output : afb().outputSignals())
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

	bool UalAfb::getAfbSignalByPinUuid(QUuid pinUuid, AfbSignal* afbSignal) const
	{
		TEST_PTR_RETURN_FALSE(afbSignal);

		for(const SchemaPin& inPin : inputs())
		{
			if (inPin.guid() == pinUuid)
			{
				return getAfbSignalByPin(inPin, afbSignal);
			}
		}

		for(const SchemaPin& outPin : outputs())
		{
			if (outPin.guid() == pinUuid)
			{
				return getAfbSignalByPin(outPin, afbSignal);
			}
		}

		LOG_INTERNAL_ERROR_MSG(m_log, QString(tr("Can't find signal with pin Uuid = %1 in AFB %2")).
										arg(pinUuid.toString()).arg(caption()));
		return false;
	}

	bool UalAfb::getAfbSignalByCaption(const QString& pinCaption, AfbSignal* afbSignal) const
	{
		TEST_PTR_RETURN_FALSE(afbSignal);

		for(const SchemaPin& inPin : inputs())
		{
			if (inPin.caption() == pinCaption)
			{
				return getAfbSignalByPin(inPin, afbSignal);
			}
		}

		for(const SchemaPin& outPin : outputs())
		{
			if (outPin.caption() == pinCaption)
			{
				return getAfbSignalByPin(outPin, afbSignal);
			}
		}

		LOG_INTERNAL_ERROR_MSG(m_log, QString(tr("Can't find signal with pin caption = %1 in AFB %2")).
										arg(pinCaption).arg(caption()));
		return false;
	}

	bool UalAfb::setParamValueByCaption(const QString& paramCaption, const QVariant& value)
	{
		for(AfbParamValue& param : m_paramValuesArray)
		{
			if (param.caption() == paramCaption)
			{
				param.setValue(value);
				return true;
			}
		}

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

	bool UalAfb::checkUnsignedInt(const AfbParamValue& paramValue)
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

	bool UalAfb::checkUnsignedInt16(const AfbParamValue& paramValue)
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

	bool UalAfb::checkUnsignedInt32(const AfbParamValue& paramValue)
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

	bool UalAfb::checkSignedInt32(const AfbParamValue& paramValue)
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

	bool UalAfb::checkFloat32(const AfbParamValue& paramValue)
	{
		if (paramValue.isFloat32())
		{
			return true;
		}

		// Parameter '%1' of AFB '%2' must have type 32-bit Float.
		//
		m_log->errALC5050(paramValue.opName(), caption(), guid(), label(), schemaID());

		return false;
	}

	QString UalAfb::lmDescriptionName() const
	{
		if (m_compiler == nullptr)
		{
			Q_ASSERT(false);
			return QString();
		}

		return m_compiler->lmDescriptionName();
	}

	// ---------------------------------------------------------------------------------------
	//
	// UalAfbs class implementation, this class owns all created UalAfb
	//
	// ---------------------------------------------------------------------------------------

	UalAfbs::UalAfbs()
	{
	}

	UalAfbs::~UalAfbs()
	{
		clear();
	}

	void UalAfbs::clear()
	{
		for(UalAfb* afb : m_afbs)
		{
			delete afb;
		}

		m_afbs.clear();
		m_guidToAfb.clear();
		m_fbNumber = 1;
	}

	UalAfb* UalAfbs::insert(UalAfb* afb)
	{
		TEST_PTR_RETURN_NULLPTR(afb);

		afb->setNumber(m_fbNumber);

		m_fbNumber++;

		m_afbs.push_back(afb);

		Q_ASSERT(m_guidToAfb.contains(afb->guid()) == false);

		m_guidToAfb.emplace(afb->guid(), afb);

		return afb;
	}

	UalAfb* UalAfbs::getAfb(const QUuid& afbGuid) const
	{
		return getValueOrNullptr(m_guidToAfb, afbGuid);
	}

	bool UalAfbs::contains(const QUuid& afbGuid) const
	{
		return m_guidToAfb.contains(afbGuid);
	}

	std::vector<UalAfb*>::iterator UalAfbs::begin()
	{
		return m_afbs.begin();
	}

	std::vector<UalAfb*>::const_iterator UalAfbs::begin() const
	{
		return m_afbs.begin();
	}

	std::vector<UalAfb*>::iterator UalAfbs::end()
	{
		return m_afbs.end();
	}

	std::vector<UalAfb*>::const_iterator UalAfbs::end() const
	{
		return m_afbs.end();
	}

	std::tuple<bool, bool, double> UalAfbs::getUalAfbParamValue(const QString& itemLabel, const QString& paramName) const
	{
		static const std::tuple<bool, bool, double> NOT_FOUND_RESULT(false, false, 0);

		if (m_afbs.empty())
		{
			return NOT_FOUND_RESULT;
		}

		if (m_labelHashToAfb.empty() == true)
		{
			for(const UalAfb* afb : m_afbs)
			{
				TEST_PTR_CONTINUE(afb);
				m_labelHashToAfb.emplace(calcHash(afb->label()), afb);
			}
		}

		auto it = m_labelHashToAfb.find(calcHash(itemLabel));

		if (it == m_labelHashToAfb.end())
		{
			return NOT_FOUND_RESULT;
		}

		bool itemFound = true;
		bool paramFound = false;
		double paramValue = 0;

		const UalAfb* afb = it->second;

		const AfbParam* param = nullptr;

		param = afb->getParamByCaption(paramName, true);

		if (param == nullptr)
		{
			param = afb->getParamByCaption(paramName, false);

			if (param == nullptr)
			{
				param = afb->getParamByOpName(paramName);
			}
		}

		if (param != nullptr)
		{
			paramFound = true;
			paramValue = param->afbParamValue().value().toDouble();
		}

		return std::tuple(itemFound, paramFound, paramValue);
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
		m_refSignals.clear();
	}

	bool UalSignal::createRegularSignal(const UalItem* ualItem, const QUuid& outPinGuid, AppSignal* s)
	{
		// ualItem can be == nullptr!!!
		//
		setSourceUalItem(ualItem, outPinGuid);

		if (s == nullptr )
		{
			assert(false);
			return false;
		}

		appendRefAppSignal(s, false);

		// input and tuning signals have already been computed
		//
		if (isSource() == true)
		{
			setComputed();
		}

		return true;
	}

	bool UalSignal::createConstSignal(	const QString& lmEquipmentID,
										const UalItem* ualItem,
										const QString& constSignalID,
										E::SignalType constSignalType,
										E::AnalogAppSignalFormat constAnalogFormat,
										AppSignal** autoSignalPtr)
	{
		TEST_PTR_RETURN_FALSE(ualItem);
		TEST_PTR_RETURN_FALSE(autoSignalPtr);

		const SchemaConst* ualConst = ualItem->schemaConst();

		const std::vector<SchemaPin>& outs = ualConst->outputs();

		if (outs.size() != 1)
		{
			Q_ASSERT(false);
			return false;
		}

		setSourceUalItem(ualItem, outs[0].guid());

		TEST_PTR_RETURN_FALSE(ualConst);

		// const UalSignal creation

		AppSignal* autoSignal = *autoSignalPtr = new AppSignal;

		autoSignal->setEquipmentID(lmEquipmentID);
		autoSignal->setLmEquipmentID(lmEquipmentID);
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

		appendRefAppSignal(autoSignal, false);

		// set Const signal fields
		//
		m_isConst = true;

		if (ualConst->discreteValue().hasReference() == true ||
			ualConst->signedInt32Value().hasReference() == true ||
			ualConst->floatValue().hasReference() == true)
		{
			// All references must be resolved before this step
			//
			Q_ASSERT(ualConst->discreteValue().hasReference() == false);
			Q_ASSERT(ualConst->signedInt32Value().hasReference() == false);
			Q_ASSERT(ualConst->floatValue().hasReference() == false);

			return false;
		}

		m_constDiscreteValue = ualConst->discreteNativeValue();
		m_constIntValue = ualConst->signedInt32NativeValue();
		m_constFloatValue = ualConst->floatNativeValue();

		setComputed();

		return true;
	}

	bool UalSignal::createAutoSignal(const QString& lmEquipmentID,
									 const UalItem* ualItem,
									 const QUuid& outPinGuid,
									 const QString& signalID,
									 E::SignalType signalType,
									 E::AnalogAppSignalFormat analogFormat,
									AppSignal** autoSignalPtr)
	{
		TEST_PTR_RETURN_FALSE(ualItem);
		TEST_PTR_RETURN_FALSE(autoSignalPtr);

		setSourceUalItem(ualItem, outPinGuid);

		m_isAutoSignal = true;

		// analog or discrete auto UalSignal creation

		AppSignal* autoSignal = *autoSignalPtr = new AppSignal;

		autoSignal->setEquipmentID(lmEquipmentID);
		autoSignal->setLmEquipmentID(lmEquipmentID);
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

		appendRefAppSignal(autoSignal, false);

		return true;
	}

	bool UalSignal::createBusParentSignal(const QString& lmEquipmentID,
											const UalItem* ualItem,
											const QUuid& outPinGuid,
											AppSignal* appBusSignal,
											Builder::BusShared bus,
											const QString& outPinCaption,
											std::shared_ptr<Hardware::DeviceModule> lm,
											AppSignal** autoSignalPtr)
	{
		TEST_PTR_RETURN_FALSE(bus);
		TEST_PTR_RETURN_FALSE(lm);

		// at least one of this should be initialized: ualItem, appBusSignal
		//
		if (ualItem == nullptr && appBusSignal == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		setSourceUalItem(ualItem, outPinGuid);

		m_bus = bus;

		if (appBusSignal == nullptr)
		{
			// create auto bus signal
			//
			// in this case ualItem and outPinCaption should be initialized!
			//

			m_isAutoSignal = true;

			TEST_PTR_RETURN_FALSE(ualItem);
			Q_ASSERT(outPinCaption.isEmpty() == false);
			TEST_PTR_RETURN_FALSE(autoSignalPtr);

			QString appSignalID = QString("%1_%2_%3_%4").
										arg(AUTO_BUS_ID_PREFIX).
										arg(lm->equipmentIdTemplate()).
										arg(ualItem->label()).
										arg(outPinCaption.toUpper());

			*autoSignalPtr = appBusSignal = new AppSignal;

			appBusSignal->setEquipmentID(lmEquipmentID);
			appBusSignal->setLmEquipmentID(lmEquipmentID);
			appBusSignal->setAppSignalID(appSignalID);
			appBusSignal->setCustomAppSignalID(appSignalID.remove("#"));
			appBusSignal->setCaption(appBusSignal->customAppSignalID());

			appBusSignal->setSignalType(E::SignalType::Bus);
			appBusSignal->setBusTypeID(bus->busTypeID());

			appBusSignal->setDataSizeW(bus->sizeW());

			appBusSignal->setAcquire(false);
		}
		else
		{
			assert(appBusSignal->equipmentID().isEmpty() == false);
			assert(appBusSignal->lmEquipmentID() == lmEquipmentID);
			assert(appBusSignal->isBus());
			assert(appBusSignal->busTypeID() == bus->busTypeID());
		}

		appendRefAppSignal(appBusSignal, false);

		return true;
	}

	bool UalSignal::appendRefAppSignal(AppSignal* s, bool isOptoSignal)
	{
		if (s == nullptr)
		{
			assert(false);
			return false;
		}

		s->setAutoSignal(m_isAutoSignal);

		for(AppSignal* pesentSignal : m_refSignals)
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
			AppSignal* first = m_refSignals[0];

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

	bool UalSignal::appendBusChildRefSignals(const QString& busSignalID, AppSignal* s)
	{
		UalSignal* childSignal = m_busChildSignals.value(busSignalID, nullptr);

		if (childSignal == nullptr)
		{
			assert(false);
			return false;
		}

		return childSignal->appendRefAppSignal(s, false);
	}

	Address16 UalSignal::ioBufAddr() const
	{
		if (m_isInput == true)
		{
			AppSignal* inSignal = getInputSignal();

			if (inSignal == nullptr)
			{
				assert(false);
				return Address16();
			}

			return inSignal->ioBufAddr();
		}

		if (m_isOutput == true)
		{
			AppSignal* outSignal = getOutputSignal();

			if (outSignal == nullptr)
			{
				assert(false);
				return Address16();
			}

			return outSignal->ioBufAddr();
		}

		if (m_isOptoSignal == true)
		{
			AppSignal* s = signal();

			if (s == nullptr)
			{
				assert(false);
				return Address16();
			}

			return s->ioBufAddr();
		}

		return Address16();
	}

	bool UalSignal::checkIoBufAddr() const
	{
		return ioBufAddr().isValid();
	}

	AppSignal* UalSignal::signal() const
	{
		if (m_refSignals.count() < 1)
		{
			assert(false);
			return nullptr;
		}

		return m_refSignals[0];
	}

	E::SignalInOutType UalSignal::inOutType() const
	{
		if (isBusChild() == true && isFrombusConversionRequired() == true)
		{
			// bus child signal that required frombus conversion
			// will be placed in LM memory as INTERNAL signal
			// and conversion code will be generated
			//
			return E::SignalInOutType::Internal;
		}

		return m_refSignals[0]->inOutType();
	}

	bool UalSignal::isCompatible(const AppSignal* s, IssueLogger* log) const
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

	bool UalSignal::isCanBeConnectedTo(const UalItem& ualItem,
									   const AfbSignal& afbSignal,
									   IssueLogger* log) const
	{
		TEST_PTR_RETURN_FALSE(log);

		if (m_refSignals.count() < 1 || m_refSignals[0] == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		if (afbSignal.isBus() == true)
		{
			if(isBus() == true)
			{
				// bus signal connection to bus input checking
				//
				TEST_PTR_RETURN_FALSE(m_bus);

				switch(afbSignal.busDataFormat())
				{
				case E::BusDataFormat::Discrete:

					if (m_bus->busDataFormat() == E::BusDataFormat::Discrete)
					{
						return true;
					}

					// Non-discrete busses is not allowed on pin '%1'. (Item %2, logic schema %3).
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

		TEST_PTR_RETURN_FALSE(m_refSignals[0]);

		return m_refSignals[0]->isCompatibleFormat(afbSignal.type(), afbSignal.dataFormat(), afbSignal.size(), afbSignal.byteOrder());
	}

	bool UalSignal::isCompatible(BusShared bus, const Builder::BusSignal& busSignal, IssueLogger* log) const
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
			return m_refSignals[0]->isCompatibleFormat(busSignal.signalType, busSignal.inOutAnalogFormat, E::ByteOrder::BigEndian);

		case E::SignalType::Bus:

			if (isDiscrete() == true &&
				(bus->busDataFormat() == E::BusDataFormat::Discrete || bus->busDataFormat() == E::BusDataFormat::Mixed))
			{
				return true;
			}

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

	bool UalSignal::isCanBeConnectedTo(const UalSignal* destSignal, IssueLogger* log) const
	{
		// *this - is source signal
		//
		if (isDiscrete() == true && destSignal->isBus() == true)
		{
			BusShared bus = destSignal->bus();

			TEST_PTR_RETURN_FALSE(bus);

			if (bus->busDataFormat() == E::BusDataFormat::Discrete || bus->busDataFormat() == E::BusDataFormat::Mixed)
			{
				return true;
			}
		}

		return isCompatible(destSignal->signal(), log);
	}

	void UalSignal::setReceivedOptoAppSignalID(const QString& recvAppSignalID, const SchemaReceiver* ualReceiver)
	{
		m_receivedOptoAppSignalID = recvAppSignalID;
		m_ualReceiver = ualReceiver;
		m_isOptoSignal = true;
	}

	bool UalSignal::anyParentBusIsAcquired() const
	{
		if (m_parentBusSignal == nullptr)
		{
			// this is top level bus parent signal
			//
			return isAcquired();
		}

		if (m_parentBusSignal->isAcquired() == true)
		{
			return true;
		}

		return m_parentBusSignal->anyParentBusIsAcquired();
	}

    bool UalSignal::setLoopback(std::shared_ptr<Loopback> loopback)
	{
		auto p = m_loopbacks.insert(loopback);

		if (p.second == false)
		{
			// if false - this is a reassigning of loopback, why?
			//
			Q_ASSERT(false);
			return false;
		}

		return true;
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

	Address16 UalSignal::ualAddr() const
	{
		Q_ASSERT(isConst() == false);
		Q_ASSERT(isHeapPlaced() == false);

		return m_ualAddr;
	}

	Address16 UalSignal::ualAddrWithoutChecks() const
	{
		return m_ualAddr;
	}

	bool UalSignal::setUalAddr(const Address16& ualAddr)
	{
		if (m_isConst == true)
		{
			Q_ASSERT(false);					// for Const signals ualAddr isn't assigned
			return false;
		}

		if (m_isHeapPlaced == true)
		{
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(ualAddr.isValid() == true);

		if (m_ualAddr.isValid() == true && isBusChild() == true)
		{
			return true;			// ualAddress of bus child signal is allredy set, its ok
		}

		if (m_ualAddr.isValid() == true && isBusChild() == false)
		{
			Q_ASSERT(false);				// why and where m_ualAddr is already set???
			return false;
		}

		m_ualAddr = ualAddr;

		// set same ual address for all associated signals

		for(AppSignal* s : m_refSignals)
		{
			if (s->ualAddrIsValid() == true)
			{
				Q_ASSERT(s->ualAddr() == ualAddr);
			}
			else
			{
				s->setUalAddr(ualAddr);
			}
		}

		if (isBus() == false)
		{
			return true;
		}

		if (m_bus == nullptr)
		{
			Q_ASSERT(false);				// m_bus can't be null
			return false;
		}

		Q_ASSERT(ualAddr.bit() == 0);		// bus must be aligned to word

		bool result = true;

		for(const BusSignal& busSignal : m_bus->busSignals())
		{
			UalSignal* childSignal = m_busChildSignals.value(busSignal.signalID);

			if (childSignal == nullptr)
			{
				Q_ASSERT(false);
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

	bool UalSignal::ualAddrIsValid() const
	{
		return m_ualAddr.isValid();
	}

	bool UalSignal::checkUalAddr() const
	{
		if (isConst() == true)
		{
			Q_ASSERT(m_ualAddr.isValid() == false);			// UAL addr shouldn't be set for Const signals
			return true;
		}

		if (isHeapPlaced() == true)
		{
			Q_ASSERT(m_ualAddr.isValid() == false);			// UAL addr shouldn't be set for heap placed signals
			return true;
		}

		return m_ualAddr.isValid();
	}

	bool UalSignal::setRegBufAddr(const Address16& regBufAddr)
	{
		assert(regBufAddr.isValid() == true);

		if (m_regBufAddr.isValid() == true)
		{
			assert(false);				// m_regBufAddr is already set
			return false;
		}

		m_regBufAddr = regBufAddr;

		// set same regBufAddr for all associated acquired signals

		for(AppSignal* s : m_refSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false)
			{
				continue;
			}

			assert(s->regBufAddr().isValid() == false);

			s->setRegBufAddr(regBufAddr);
		}

		if (isBus() == false)
		{
			return true;
		}

		if (m_bus == nullptr)
		{
			Q_ASSERT(false);				// m_bus can't be null
			return false;
		}

		bool result = true;

		for(const BusSignal& busSignal : m_bus->busSignals())
		{
			UalSignal* childSignal = m_busChildSignals.value(busSignal.signalID);

			if (childSignal == nullptr)
			{
				Q_ASSERT(false);
				result = false;
				continue;
			}

			if (childSignal->isFrombusConversionRequired() == true)
			{
				// this bus child signal will be aquired as converted analog internal signal
				continue;
			}

			int busBitAddr = regBufAddr.bitAddress();
			int busSignalBitAddr = busSignal.inbusAddr.bitAddress();

			Address16 addr(0, 0);

			addr.addBit(busBitAddr + busSignalBitAddr);

			result &= childSignal->setRegBufAddr(addr);
		}

		return result;
	}

	bool UalSignal::checkRegBufAddr() const
	{
		return m_regBufAddr.isValid();
	}

	bool UalSignal::setRegValueAddr(const Address16& regValueAddr)
	{
		assert(regValueAddr.isValid() == true);

		if (m_regValueAddr.isValid() == true)
		{
			assert(false);				// m_regValueAddr is already set
			return false;
		}

		m_regValueAddr = regValueAddr;

		// set same regBufAddr for all associated acquired signals

		for(AppSignal* s : m_refSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->isAcquired() == false)
			{
				continue;
			}

			Q_ASSERT(s->regValueAddr().isValid() == false ||
					 (s->regValueAddr().isValid() == true && s->regValueAddr() == regValueAddr));

			s->setRegValueAddr(regValueAddr);
		}

		if (isBus() == false)
		{
			return true;
		}

		if (m_bus == nullptr)
		{
			Q_ASSERT(false);				// m_bus can't be null
			return false;
		}

		bool result = true;

		for(const BusSignal& busSignal : m_bus->busSignals())
		{
			UalSignal* childSignal = m_busChildSignals.value(busSignal.signalID);

			if (childSignal == nullptr)
			{
				Q_ASSERT(false);
				result = false;
				continue;
			}

			if (childSignal->isFrombusConversionRequired() == true)
			{
				// this bus child signal will be aquired as converted analog internal signal
				continue;
			}

			int busBitAddr = regValueAddr.bitAddress();
			int busSignalBitAddr = busSignal.inbusAddr.bitAddress();

			Address16 addr(0, 0);

			addr.addBit(busBitAddr + busSignalBitAddr);

			result &= childSignal->setRegValueAddr(addr);
		}

		return true;
	}

	void UalSignal::sortRefSignals()
	{
		// sorting m_refSignals by appSignalID ascending
		//
		qsizetype count = m_refSignals.count();

		for(qsizetype i = 0; i < count - 1; i++)
		{
			for(qsizetype k = i + 1; k < count; k++)
			{
				if (m_refSignals[i]->appSignalID() > m_refSignals[k]->appSignalID())
				{
					AppSignal* tmp = m_refSignals[i];
					m_refSignals[i] = m_refSignals[k];
					m_refSignals[k] = tmp;
				}
			}
		}
	}

	AppSignal* UalSignal::getInputSignal() const
	{
		AppSignal* inputSignal = nullptr;

		for(AppSignal* s : m_refSignals)
		{
			if (s->isInput() == true)
			{
				inputSignal = s;
				break;
			}
		}

		return inputSignal;
	}

	AppSignal* UalSignal::getOutputSignal() const
	{
		AppSignal* outputSignal = nullptr;

		for(AppSignal* s : m_refSignals)
		{
			if (s->isOutput() == true)
			{
				outputSignal = s;
				break;
			}
		}

		return outputSignal;
	}

	AppSignal* UalSignal::getTunableSignal() const
	{
		AppSignal* tunableSignal = nullptr;

		for(AppSignal* s : m_refSignals)
		{
			if (s->enableTuning() == true)
			{
				tunableSignal = s;
				break;
			}
		}

		return tunableSignal;
	}

	QVector<AppSignal*> UalSignal::getAnalogOutputSignals() const
	{
		QVector<AppSignal*> analogOutputs;

		if (isAnalog() == false)
		{
			assert(false);
			return analogOutputs;
		}

		for(AppSignal* s : m_refSignals)
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

		for(AppSignal* s : m_refSignals)
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

		for(AppSignal* s : m_refSignals)
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

		for(AppSignal* s : m_refSignals)
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

		const SchemaReceiver* ualReceiver = m_ualItem->schemaReceiver();

		if (ualReceiver == nullptr)
		{
			assert(false);
			return QString();
		}

		return ualReceiver->connectionIds();
	}

	void UalSignal::setSourceUalItem(const UalItem* ualItem, const QUuid& outPinGuid)
	{
		if (m_ualItem != nullptr)
		{
			return;		// only one source ualItem.outPinGuid can be exist
		}

		if (ualItem == nullptr ||
			outPinGuid.isNull())
		{
			return;
		}

		const SchemaPin* pin = ualItem->getPin(outPinGuid);

		if (pin == nullptr)
		{
			return;
		}

		if (pin->IsOutput() == false)
		{
			return;
		}

		m_ualItem = ualItem;
		m_outPinGuid = outPinGuid;
	}

	const UalItem* UalSignal::ualItem() const
	{
		return m_ualItem;
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

	QString UalSignal::ualItemLabelOutPinCaption() const
	{
		QString caption;

		if (m_ualItem != nullptr)
		{
			caption += m_ualItem->label();

			if (m_outPinGuid.isNull() == false)
			{
				const SchemaPin* pin = m_ualItem->getPin(m_outPinGuid);

				if (pin != nullptr && pin->caption().isEmpty() == false)
				{
					caption += Separator::DOT;
					caption += pin->caption();
				}
			}
		}

		return caption;
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

		ualSignal->setParentBusSignal(this);
		ualSignal->setAutoSignal(m_isAutoSignal);

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

		for(AppSignal* refSignal : m_refSignals)
		{
			TEST_PTR_CONTINUE(refSignal);

			refSignal->setAcquire(acquired);
		}
	}

	bool UalSignal::addStateFlagSignal(const QString& signalWithFlagID, E::AppSignalStateFlagType flagType, const QString& flagSignalID, IssueLogger* log)
	{
		bool result = true;

		bool signalWithFlagID_isFound = false;

		for(AppSignal* s : m_refSignals)
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

	void UalSignal::preliminarySetHeapPlaced(int expectedHeapReadsCount)
	{
		m_expectedHeapReadsCount = expectedHeapReadsCount;
	}

	bool UalSignal::canBePlacedInHeap() const
	{
		return	m_isAcquired == false &&
				isLoopbackSource() == false &&
				m_expectedHeapReadsCount > 0;
	}

	void UalSignal::setHeapPlaced()
	{
		Q_ASSERT(m_expectedHeapReadsCount > 0);
		m_isHeapPlaced = true;
	}

	void UalSignal::resetHeapPlaced()
	{
		m_isHeapPlaced = false;
		m_expectedHeapReadsCount = 0;
	}

	int UalSignal::expectedHeapReadsCount() const
	{
		Q_ASSERT(m_isHeapPlaced == true);

		return m_expectedHeapReadsCount;
	}

	void UalSignal::setAutoSignal(bool autoSignal)
	{
		m_isAutoSignal = autoSignal;

		for(UalSignal* busChildSignal : m_busChildSignals)
		{
			TEST_PTR_CONTINUE(busChildSignal);

			busChildSignal->setAutoSignal(autoSignal);
		}

		for(AppSignal* refAppSignal : m_refSignals)
		{
			TEST_PTR_CONTINUE(refAppSignal);

			refAppSignal->setAutoSignal(autoSignal);
		}
	}

	// ---------------------------------------------------------------------------------------
	//
	// UalSignalsMap class implementation
	//
	// ---------------------------------------------------------------------------------------

	UalSignals::UalSignals(ModuleLogicCompiler& compiler, IssueLogger* log) :
		m_compiler(compiler),
		m_log(log),
		m_discreteSignalsHeap(SIZE_1BIT, compiler.generateExtraDebugInfo(), log),
		m_analogAndBusSignalsHeap(SIZE_16BIT, compiler.generateExtraDebugInfo(), log)
	{
	}

	UalSignals::~UalSignals()
	{
		clear();
	}

	void UalSignals::clear()
	{
		for(UalSignal* ualSignal : m_signals)
		{
			DELETE_IF_NOT_NULL(ualSignal);
		}

		m_signals.clear();
		m_idToSignalMap.clear();
		m_pinToSignalMap.clear();
		//m_signalToPinsMap.clear();
	}

	std::vector<UalSignal*>::iterator UalSignals::begin()
	{
		return m_signals.begin();
	}

	std::vector<UalSignal*>::const_iterator UalSignals::begin() const
	{
		return m_signals.begin();
	}

	std::vector<UalSignal*>::iterator UalSignals::end()
	{
		return m_signals.end();
	}

	std::vector<UalSignal*>::const_iterator UalSignals::end() const
	{
		return m_signals.end();
	}

	UalSignal* UalSignals::get(const QString& appSignalID) const
	{
		return getValueOrNullptr(m_idToSignalMap, appSignalID);
	}

	bool UalSignals::contains(const QString& appSignalID) const
	{
		return m_idToSignalMap.contains(appSignalID);
	}

	UalSignal* UalSignals::get(const QUuid& pinUuid) const
	{
		return getValueOrNullptr(m_pinToSignalMap, pinUuid);
	}

	bool UalSignals::contains(QUuid pinUuid) const
	{
		return m_pinToSignalMap.contains(pinUuid);
	}

	UalSignal* UalSignals::get(const AppSignal* appSignal) const
	{
		return getValueOrNullptr(m_ptrToSignalMap, appSignal);
	}

	bool UalSignals::contains(const UalSignal* ualSignal) const
	{
		return m_signalSet.contains(ualSignal);
	}

	UalSignal* UalSignals::createSignal(AppSignal* appSignal)
	{
		TEST_PTR_RETURN_NULLPTR(appSignal);

		switch(appSignal->signalType())
		{
		case E::SignalType::Discrete:
		case E::SignalType::Analog:
			return createSignal(appSignal, nullptr, QUuid());

		case E::SignalType::Bus:
			return createBusParentSignal(appSignal);

		default:
			Q_ASSERT(false);
		}

		return nullptr;
	}

	UalSignal* UalSignals::createSignal(AppSignal* appSignal, const UalItem* ualItem, QUuid outPinUuid)
	{
		// ualItem can be nullptr!!!

		if (appSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return nullptr;
		}

		UalSignal* ualSignal = get(appSignal->appSignalID());

		if (ualSignal != nullptr)
		{
			// signal already in map
			//
			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			ualSignal->setSourceUalItem(ualItem, outPinUuid);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		// create new signal
		//
		ualSignal = new UalSignal;

		bool result = ualSignal->createRegularSignal(ualItem, outPinUuid, appSignal);

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

	UalSignal* UalSignals::createConstSignal(const UalItem* ualItem,
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

		UalSignal* ualSignal = get(constSignalID);

		if (ualSignal != nullptr)
		{
			// const already in map
			//
			assert(false);

			assert(m_pinToSignalMap.contains(outPinUuid) == false);

			appendPinRefToSignal(outPinUuid, ualSignal);

			return ualSignal;
		}

		const SchemaConst* ualConst = ualItem->schemaConst();

		if (ualConst == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		// create new const signal
		//
		ualSignal = new UalSignal;

		AppSignal* autoSignalPtr = nullptr;

		bool result = ualSignal->createConstSignal(m_compiler.lmEquipmentID(),
													ualItem,
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
			m_compiler.signalSet()->append(autoSignalPtr, lm());
		}
		else
		{
			assert(false);
		}

		return ualSignal;
	}

	UalSignal* UalSignals::createAutoSignal(const UalItem* ualItem, QUuid outPinUuid,
											   const AfbSignal& templateOutAfbSignal,
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

	UalSignal* UalSignals::createAutoSignal(const UalItem* ualItem, QUuid outPinUuid, const AppSignal& templateSignal)
	{
		return privateCreateAutoSignal(ualItem, outPinUuid, templateSignal.signalType(), templateSignal.analogSignalFormat(), -1);
	}

	UalSignal* UalSignals::createBusParentSignal(AppSignal* appBusSignal)
	{
		TEST_PTR_LOG_RETURN_NULLPTR(appBusSignal, m_log);

		if (appBusSignal->isBus() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		BusShared bus = m_compiler.getBusShared(appBusSignal->busTypeID());

		if (bus == nullptr)
		{
			// Bus type ID %1 of signal %2 is undefined.
			//
			m_log->errALC5092(appBusSignal->busTypeID(), appBusSignal->appSignalID());
			return nullptr;
		}

		return createBusParentSignal(appBusSignal, bus, nullptr, QUuid(), QString());
	}

	UalSignal* UalSignals::createBusParentSignal(AppSignal* appBusSignal,
													BusShared bus,
													const UalItem* ualItem,
													QUuid outPinUuid,
													const QString& outPinCaption)
	{
		// at least one of this should be initialized: ualItem, appBusSignal
		//
		if (ualItem == nullptr && appBusSignal == nullptr)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		if ((appBusSignal != nullptr && appBusSignal->isBus() == false) || (bus == nullptr))
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
			return nullptr;
		}

		//

		if (appBusSignal != nullptr)
		{
			UalSignal* ualSignal = get(appBusSignal->appSignalID());

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

		AppSignal* autoSignalPtr = nullptr;

		bool result = busParentSignal->createBusParentSignal(m_compiler.lmEquipmentID(), ualItem, outPinUuid,
															 appBusSignal, bus, outPinCaption,
															 m_compiler.getLmSharedPtr(), &autoSignalPtr);

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
			AppSignal* sChild = m_compiler.signalSet()->appendBusChildSignal(*busParentSignal->signal(), bus, busSignal, lm());

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

					busChildSignal = createBusParentSignal(sChild, childBus, ualItem, QUuid(), busSignal.caption);
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
			m_compiler.signalSet()->append(autoSignalPtr, lm());
		}

		return busParentSignal;
	}

	bool UalSignals::appendRefPin(const UalItem* ualItem, QUuid pinUuid, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		ualSignal->setSourceUalItem(ualItem, pinUuid);

		if (m_signalSet.contains(ualSignal) == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// ualSignal must exists!
			return false;
		}

		UalSignal* existsSignal = get(pinUuid);

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

	bool UalSignals::appendRefSignal(AppSignal* s, UalSignal* ualSignal)
	{
		if (ualSignal == nullptr || s == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (m_signalSet.contains(ualSignal) == false)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);			// ualSignal must exists!
			return false;
		}

		UalSignal* existsSignal = get(s);

		if (existsSignal != nullptr)
		{
			if (existsSignal == ualSignal)
			{
				// ref to same signal, its Ok
				//
				return true;
			}

			QString msg = QString("Signal %1 try appendRef to %2 and %3 ual signals").
					arg(s->appSignalID()).arg(ualSignal->refSignalIDsJoined()).arg(existsSignal->refSignalIDsJoined());

			LOG_INTERNAL_ERROR_MSG(m_log, msg);

			return false;
		}

		existsSignal = get(s->appSignalID());

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

		bool result = ualSignal->appendRefAppSignal(s, false);

		if (result == false)
		{
			return false;
		}

		m_idToSignalMap.emplace(s->appSignalID(), ualSignal);

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
			AppSignal* newSignal = m_compiler.signalSet()->appendBusChildSignal(*s, bus, busSignal, lm());

			result &= ualSignal->appendBusChildRefSignals(busSignal.signalID, newSignal);
		}

		return result;
	}

	bool UalSignals::getReport(QStringList& report) const
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
			UalSignal* ualSignal = get(signalID);

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

			if (ualSignal->isHeapPlaced() == true)
			{
				str.append("heap;-1;-1;");		// no ualAddr
			}
			else
			{
				if (ualSignal->isConst() == true)
				{
					str.append("const;-1;-1;");		// no ualAddr
				}
				else
				{
					str.append("static;");
					str.append(QString::number(ualSignal->ualAddr().offset()));
					str += ";";
					str.append(QString::number(ualSignal->ualAddr().bit()));
					str += ";";
				}
			}

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

			report.append(str);
		}

		return true;
	}

	void UalSignals::initDiscreteSignalsHeap(int startAddrW, int sizeW)
	{
		m_discreteSignalsHeap.initHeap(startAddrW, sizeW);
	}

	int UalSignals::getDiscreteSignalsHeapSizeW() const
	{
		return m_discreteSignalsHeap.getHeapUsedSizeW();
	}

	void UalSignals::initAnalogAndBusSignalsHeap(int startAddrW, int sizeW)
	{
		m_analogAndBusSignalsHeap.initHeap(startAddrW, sizeW);
	}

	int UalSignals::getAnalogAndBusSignalsHeapSizeW() const
	{
		return m_analogAndBusSignalsHeap.getHeapUsedSizeW();
	}

	Address16 UalSignals::getSignalWriteAddress(const UalSignal& ualSignal)
	{
		if (ualSignal.isHeapPlaced() == false)
		{
			return ualSignal.ualAddr();
		}

		if (ualSignal.ualAddrIsValid() == true)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(m_log, QString("For heap placed signal %1 ualAddr should NOT be set").arg(ualSignal.appSignalID()));
			return Address16();
		}

		if (ualSignal.isDiscrete() == true)
		{
			return m_discreteSignalsHeap.getAddressForWrite(ualSignal);
		}

		return m_analogAndBusSignalsHeap.getAddressForWrite(ualSignal);
	}

	Address16 UalSignals::getSignalReadAddress(const UalSignal& ualSignal, bool decrementReadCount)
	{
		if (ualSignal.isHeapPlaced() == false)
		{
			Q_ASSERT(ualSignal.ualAddrIsValid() == true);
			return ualSignal.ualAddr();
		}

		Q_ASSERT(ualSignal.ualAddrIsValid() == false);		// for heap placed signals ualAddr should not be set

		if (ualSignal.isDiscrete() == true)
		{
			return m_discreteSignalsHeap.getAddressForRead(ualSignal, decrementReadCount);
		}

		return m_analogAndBusSignalsHeap.getAddressForRead(ualSignal, decrementReadCount);
	}

	void UalSignals::disposeSignalsInHeaps(const std::set<const UalSignal*>& flagsSignals)
	{
		for(UalSignal* ualSignal : *this)
		{
			TEST_PTR_CONTINUE(ualSignal);

			if (flagsSignals.contains(ualSignal) == true)
			{
				// any signal used in flags processing can't be placed in heap
				//
				ualSignal->resetHeapPlaced();
				continue;
			}

			if (ualSignal->canBePlacedInHeap() == false)
			{
				ualSignal->resetHeapPlaced();
				continue;
			}

			ualSignal->setHeapPlaced();

			switch(ualSignal->signalType())
			{
			case E::SignalType::Discrete:
				m_discreteSignalsHeap.appendItem(*ualSignal, ualSignal->expectedHeapReadsCount());
				break;

			case E::SignalType::Analog:
			case E::SignalType::Bus:
				m_analogAndBusSignalsHeap.appendItem(*ualSignal, ualSignal->expectedHeapReadsCount());
				break;

			default:

				Q_ASSERT(false);
			}
		}
	}

	bool UalSignals::finalizeHeaps()
	{
		bool result = true;

		result &= m_discreteSignalsHeap.finalizeHeap();
		result &= m_analogAndBusSignalsHeap.finalizeHeap();

		return result;
	}

	void UalSignals::getHeapsLog(QStringList* log) const
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

	std::shared_ptr<Hardware::DeviceModule> UalSignals::lm() const
	{
		return m_compiler.getLmSharedPtr();
	}

	UalSignal* UalSignals::privateCreateAutoSignal(const UalItem* ualItem,
											   QUuid outPinUuid,
											   E::SignalType signalType,
											   E::AnalogAppSignalFormat analogFormat,
											   std::optional<int> expectedReadCount)
	{
		TEST_PTR_LOG_RETURN_NULLPTR(ualItem, m_log);

		const SchemaPin* outPin = ualItem->getPin(outPinUuid);

		TEST_PTR_LOG_RETURN_NULLPTR(outPin, m_log);

		QString signalID = QString("%1_%2_%3_%4").
								arg(UalSignal::AUTO_SIGNAL_ID_PREFIX).
								arg(m_compiler.lmEquipmentID()).
								arg(ualItem->label()).
								arg(outPin->caption());

		signalID = signalID.toUpper().remove(QRegularExpression("[^#A-Z0-9_]"));

		UalSignal* ualSignal = get(signalID);

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

		AppSignal* autoSignalPtr = nullptr;

		bool result = ualSignal->createAutoSignal(m_compiler.lmEquipmentID(), ualItem, outPinUuid,
												  signalID, signalType, analogFormat, &autoSignalPtr);

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
			m_compiler.signalSet()->append(autoSignalPtr, lm());
		}
		else
		{
			assert(false);
		}

		// signals heap support
		//
		if (expectedReadCount.has_value() == true &&  expectedReadCount.value() > 0)
		{
			ualSignal->preliminarySetHeapPlaced(expectedReadCount.value());
		}
		//
		// signals heap support

		return ualSignal;
	}

	bool UalSignals::insertNew(QUuid pinUuid, UalSignal* newUalSignal)
	{
		if (newUalSignal == nullptr || newUalSignal->signal() == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		if (contains(newUalSignal) == true)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		AppSignal* s = newUalSignal->signal();

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

		m_signals.push_back(newUalSignal);
		m_signalSet.insert(newUalSignal);
		m_idToSignalMap.emplace(signalID, newUalSignal);
		m_ptrToSignalMap.emplace(s, newUalSignal);

		appendPinRefToSignal(pinUuid, newUalSignal);

		return true;
	}

	void UalSignals::appendPinRefToSignal(QUuid pinUuid, UalSignal* ualSignal)
	{
		TEST_PTR_RETURN(ualSignal);

		if (pinUuid.isNull() == true)
		{
			return;							// is not an error
		}

		m_pinToSignalMap.emplace(pinUuid, ualSignal);
	}

	QString UalSignals::getAutoSignalID(const UalItem* ualItem, const SchemaPin& outputPin)
	{
		TEST_PTR_RETURN_VALUE(ualItem, QString());

		QString strID = QString("#AUTO_%1_%2").arg(ualItem->label()).arg(outputPin.caption());

		strID = strID.toUpper().remove(QRegularExpression("[^#A-Z0-9_]"));

		return strID;
	}

	bool UalSignals::getAnalogFormat(const AfbSignal& afbSignal, E::AnalogAppSignalFormat* analogFormat)
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
