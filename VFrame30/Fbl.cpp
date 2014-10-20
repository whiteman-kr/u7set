#include "Stable.h"
#include "Fbl.h"

namespace Afbl
{
	//
	//
	//							FblParamValue		
	//
	//
	bool AfbParamValue::SaveData(Proto::FblParamValue* /*message*/) const
	{
		assert(false);
		return false;

//		message->set_integralvalue(IntegralValue);
//		message->set_floatingpoint(FloatingPoint);
//		message->set_discrete(Discrete);
//		return true;
	}

	bool AfbParamValue::LoadData(const Proto::FblParamValue& /*message*/)
	{
		assert(false);
		return false;

//		IntegralValue = message.integralvalue();
//		FloatingPoint = message.floatingpoint();
//		Discrete = message.discrete();
//		return true;
	}

	//
	//
	//							FblElementParam		
	//
	//
	bool AfbElementParam::SaveData(Proto::FblElementParam* /*message*/) const
	{
		assert(false);
		return false;

//		Proto::Write(message->mutable_caption(), m_caption);
//		message->set_type(static_cast<Proto::FblParamType>(m_type));

//		m_value.SaveData(message->mutable_value());
//		m_defaultValue.SaveData(message->mutable_defaultvalue());

//		m_lowLimit.SaveData(message->mutable_lowlimit());
//		m_highLimit.SaveData(message->mutable_highlimit());

//		return true;
	}

	bool AfbElementParam::LoadData(const Proto::FblElementParam& /*message*/)
	{
		assert(false);
		return false;

//		m_caption = Proto::Read(message.caption());
//		m_type = static_cast<FblParamType>(message.type());

//		m_value.LoadData(message.value());
//		m_defaultValue.LoadData(message.defaultvalue());

//		m_lowLimit.LoadData(message.lowlimit());
//		m_highLimit.LoadData(message.highlimit());

//		return true;
	}

	//
	//
	//							CFblElementSignal		
	//
	//
	AfbElementSignal::AfbElementSignal(void)
	{
		m_type = AfbSignalType::Analog;
	}

	AfbElementSignal::~AfbElementSignal(void)
	{
	}

	// Serialization
	//
	bool AfbElementSignal::SaveData(Proto::FblElementSignal* /*message*/) const
	{
		assert(false);
		return false;

//		Proto::Write(message->mutable_caption(), m_caption);
//		message->set_type(static_cast<Proto::FblSignalType>(m_type));
//		return true;
	}
	
	bool AfbElementSignal::LoadData(const Proto::FblElementSignal& /*message*/)
	{
		assert(false);
		return false;

//		m_caption = Proto::Read(message.caption());
//		m_type = static_cast<FblSignalType>(message.type());
//		return true;
	}

	// Caption
	//
	const QString& AfbElementSignal::caption() const
	{
		return m_caption;
	}
	void AfbElementSignal::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	AfbSignalType AfbElementSignal::type() const
	{
		return m_type;
	}
	void AfbElementSignal::setType(AfbSignalType type)
	{
		m_type = type;
	}

	//
	//
	//							CFblElementParam		
	//
	//

	AfbElementParam::AfbElementParam(void)
	{
	}

	AfbElementParam::~AfbElementParam(void)
	{
	}

	// Caption
	//
	const QString& AfbElementParam::caption() const
	{
		return m_caption;
	}
	void AfbElementParam::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	AfbParamType AfbElementParam::type() const
	{
		return m_type;
	}
	void AfbElementParam::setType(AfbParamType type)
	{
		m_type = type;
	}

	// Value
	//
	const AfbParamValue& AfbElementParam::value() const
	{
		return m_value;
	}
	void AfbElementParam::setValue(const AfbParamValue& value)
	{
		m_value = value;
	}

	// Defaut Value
	//
	const AfbParamValue& AfbElementParam::defaultValue() const
	{
		return m_defaultValue;
	}
	void AfbElementParam::setDefaultValue(const AfbParamValue& defaultValue)
	{
		m_defaultValue = defaultValue;
	}

	// LowLimit
	const AfbParamValue& AfbElementParam::lowLimit() const
	{
		return m_lowLimit;
	}
	void AfbElementParam::setLowLimit(const AfbParamValue& lowLimit)
	{
		m_lowLimit = lowLimit;
	}

	// highLimit
	//
	const AfbParamValue& AfbElementParam::highLimit() const
	{
		return m_highLimit;
	}
	void AfbElementParam::setHighLimit(const AfbParamValue& highLimit)
	{
		m_highLimit = highLimit;
	}

	//
	//
	//							FblElement		
	//
	//

	AfbElement::AfbElement(void)
	{
		Init();
	}

	AfbElement::~AfbElement(void)
	{
	}

	void AfbElement::Init(void)
	{
		m_opcode = 0;
		m_guid = QUuid();
	}

	bool AfbElement::loadFromXml(const QByteArray& data)
	{
		return true;
	}

	bool AfbElement::saveToXml(QByteArray* data) const
	{
		return true;
	}

	// Serialization
	//
	bool AfbElement::SaveData(Proto::Envelope* /*message*/) const
	{
		assert(false);
		return false;

//		quint32 classnamehash = CUtils::GetClassHashCode("FblElement");
//		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

//		Proto::FblElement* pMutableFblElement = message->mutable_fblelement();

//		Proto::Write(pMutableFblElement->mutable_uuid(), m_guid);
//		Proto::Write(pMutableFblElement->mutable_strid(), m_strID);
//		Proto::Write(pMutableFblElement->mutable_caption(), m_caption);
//		pMutableFblElement->set_opcode(m_opcode);

//		for (auto signal = m_inputSignals.begin(); signal != m_inputSignals.end(); ++signal)
//		{
//			Proto::FblElementSignal* s = pMutableFblElement->mutable_inputsignals()->Add();
//			signal->SaveData(s);
//		}

//		for (auto signal = m_outputSignals.begin(); signal != m_outputSignals.end(); ++signal)
//		{
//			Proto::FblElementSignal* s = pMutableFblElement->mutable_outputsignals()->Add();
//			signal->SaveData(s);
//		}

//		for (auto param = m_params.begin(); param != m_params.end(); ++param)
//		{
//			Proto::FblElementParam* p = pMutableFblElement->mutable_params()->Add();
//			param->SaveData(p);
//		}
		
//		return true;
	}

	bool AfbElement::LoadData(const Proto::Envelope& /*message*/)
	{
		assert(false);
		return false;

//		if (message.has_fblelement() == false)
//		{
//			assert(message.has_fblelement());
//			return false;
//		}

//		const Proto::FblElement& fblelement = message.fblelement();

//		m_guid = Proto::Read(fblelement.uuid());
//		m_strID = Proto::Read(fblelement.strid());
//		m_caption = Proto::Read(fblelement.caption());
//		m_opcode = fblelement.opcode();
			
//		// Read input signals
//		//
//		m_inputSignals.clear();
//		for (int i = 0; i < fblelement.inputsignals().size(); i++)
//		{
//			Fbl::FblElementSignal s;
//			s.LoadData(fblelement.inputsignals(i));
			
//			m_inputSignals.push_back(s);
//		}

//		// Read output signals
//		//
//		m_outputSignals.clear();
//		for (int i = 0; i < fblelement.outputsignals().size(); i++)
//		{
//			Fbl::FblElementSignal s;
//			s.LoadData(fblelement.outputsignals(i));
			
//			m_outputSignals.push_back(s);
//		}

//		// Read params
//		//
//		m_params.clear();
//		for (int i = 0; i < fblelement.params().size(); i++)
//		{
//			Fbl::FblElementParam p;
//			p.LoadData(fblelement.params(i));
			
//			m_params.push_back(p);
//		}

//		return true;
	}

	AfbElement* AfbElement::CreateObject(const Proto::Envelope& /*message*/)
	{
		assert(false);
		return nullptr;

//		// Эта функция может создавать только один экземпляр
//		//
//		if (message.has_fblelement() == false)
//		{
//			assert(message.has_fblelement());
//			return nullptr;
//		}

//		FblElement* pFblElement = new FblElement();
		
//		pFblElement->LoadData(message);

//		return pFblElement;
	}

	// Properties and Data
	//

	// Guid
	//
	const QUuid& AfbElement::guid() const
	{
		return m_guid;
	}
	void AfbElement::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// StrID
	//
	const QString& AfbElement::strID() const
	{
		return m_strID;
	}
	void AfbElement::setStrID(const QString& strID)
	{
		m_strID = strID;
	}

	// Caption
	//
	const QString& AfbElement::caption() const
	{
		return m_caption;
	}
	void AfbElement::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Opcode
	//
	unsigned int AfbElement::opcode() const
	{
		return m_opcode;
	}
	void AfbElement::setOpcode(unsigned int value)
	{
		m_opcode = value;
	}

	// InputSignals
	//
	const std::vector<AfbElementSignal>& AfbElement::inputSignals() const
	{
		return m_inputSignals;
	}
	void AfbElement::setInputSignals(const std::vector<AfbElementSignal>& inputsignals)
	{
		m_inputSignals = inputsignals;
	}

	// OutputSignals
	//
	const std::vector<AfbElementSignal>& AfbElement::outputSignals() const
	{
		return m_outputSignals;
	}
	void AfbElement::setOutputSignals(const std::vector<AfbElementSignal>& outputsignals)
	{
		m_outputSignals = outputsignals;
	}

	// Params
	//
	const std::vector<AfbElementParam>& AfbElement::params() const
	{
		return m_params;
	}
	void AfbElement::setParams(const std::vector<AfbElementParam>& params)
	{
		m_params = params;
	}


	//
	//
	//				FblElementCollection - Коллекция прототипов FBL элементов
	//
	//

	AfbElementCollection::AfbElementCollection(void)
	{
		Init();
	}

	AfbElementCollection::~AfbElementCollection(void)
	{
	}

	void AfbElementCollection::Init(void)
	{
	}

	void AfbElementCollection::setElements(const std::vector<std::shared_ptr<AfbElement>>& elements)
	{
		m_elements = elements;
	}

	const std::vector<std::shared_ptr<AfbElement>>& AfbElementCollection::elements() const
	{
		return m_elements;
	}

	std::vector<std::shared_ptr<AfbElement>>* AfbElementCollection::mutable_elements()
	{
		return &m_elements;
	}

	std::shared_ptr<AfbElement> AfbElementCollection::get(const QUuid& guid) const
	{
		auto result = std::find_if(m_elements.begin(), m_elements.end(),
			[&guid](const std::shared_ptr<AfbElement>& fblelement)
			{
				return fblelement->guid() == guid;
			});
		
		return result == m_elements.end() ? std::shared_ptr<AfbElement>() : *result;
	}

}

