#include "Stable.h"
#include "Fbl.h"

namespace Fbl
{
	//
	//
	//							FblParamValue		
	//
	//
	bool FblParamValue::SaveData(::Proto::FblParamValue* message) const
	{
		message->set_integralvalue(IntegralValue);
		message->set_floatingpoint(FloatingPoint);
		message->set_discrete(Discrete);
		return true;
	}

	bool FblParamValue::LoadData(const ::Proto::FblParamValue& message)
	{
		IntegralValue = message.integralvalue();
		FloatingPoint = message.floatingpoint();
		Discrete = message.discrete();
		return true;
	}

	//
	//
	//							FblElementParam		
	//
	//
	bool FblElementParam::SaveData(::Proto::FblElementParam* message) const
	{
		VFrame30::Proto::Write(message->mutable_caption(), m_caption);
		message->set_type(static_cast<::Proto::FblParamType>(m_type));

		m_value.SaveData(message->mutable_value());
		m_defaultValue.SaveData(message->mutable_defaultvalue());

		m_lowLimit.SaveData(message->mutable_lowlimit());
		m_highLimit.SaveData(message->mutable_highlimit());

		return true;
	}

	bool FblElementParam::LoadData(const ::Proto::FblElementParam& message)
	{
		m_caption = VFrame30::Proto::Read(message.caption());
		m_type = static_cast<FblParamType>(message.type());

		m_value.LoadData(message.value());
		m_defaultValue.LoadData(message.defaultvalue());

		m_lowLimit.LoadData(message.lowlimit());
		m_highLimit.LoadData(message.highlimit());

		return true;
	}

	//
	//
	//							CFblElementSignal		
	//
	//
	FblElementSignal::FblElementSignal(void)
	{
		m_type = FblSignalType::Analog;
	}

	FblElementSignal::~FblElementSignal(void)
	{
	}

	// Serialization
	//
	bool FblElementSignal::SaveData(::Proto::FblElementSignal* message) const
	{
		VFrame30::Proto::Write(message->mutable_caption(), m_caption);
		message->set_type(static_cast<::Proto::FblSignalType>(m_type));
		return true;
	}
	
	bool FblElementSignal::LoadData(const ::Proto::FblElementSignal& message)
	{
		m_caption = VFrame30::Proto::Read(message.caption());
		m_type = static_cast<FblSignalType>(message.type());
		return true;
	}

	// Caption
	//
	const QString& FblElementSignal::caption() const
	{
		return m_caption;
	}
	void FblElementSignal::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	FblSignalType FblElementSignal::type() const
	{
		return m_type;
	}
	void FblElementSignal::setType(FblSignalType type)
	{
		m_type = type;
	}

	//
	//
	//							CFblElementParam		
	//
	//

	FblElementParam::FblElementParam(void)
	{
	}

	FblElementParam::~FblElementParam(void)
	{
	}

	// Caption
	//
	const QString& FblElementParam::caption() const
	{
		return m_caption;
	}
	void FblElementParam::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	FblParamType FblElementParam::type() const
	{
		return m_type;
	}
	void FblElementParam::setType(FblParamType type)
	{
		m_type = type;
	}

	// Value
	//
	const FblParamValue& FblElementParam::value() const
	{
		return m_value;
	}
	void FblElementParam::setValue(const FblParamValue& value)
	{
		m_value = value;
	}

	// Defaut Value
	//
	const FblParamValue& FblElementParam::defaultValue() const
	{
		return m_defaultValue;
	}
	void FblElementParam::setDefaultValue(const FblParamValue& defaultValue)
	{
		m_defaultValue = defaultValue;
	}

	// LowLimit
	const FblParamValue& FblElementParam::lowLimit() const
	{
		return m_lowLimit;
	}
	void FblElementParam::setLowLimit(const FblParamValue& lowLimit)
	{
		m_lowLimit = lowLimit;
	}

	// highLimit
	//
	const FblParamValue& FblElementParam::highLimit() const
	{
		return m_highLimit;
	}
	void FblElementParam::setHighLimit(const FblParamValue& highLimit)
	{
		m_highLimit = highLimit;
	}

	//
	//
	//							FblElement		
	//
	//

	FblElement::FblElement(void)
	{
		Init();
	}

	FblElement::~FblElement(void)
	{
	}

	void FblElement::Init(void)
	{
		m_opcode = 0;
		m_guid = QUuid();
	}

	// Serialization
	//
	bool FblElement::SaveData(::Proto::Envelope* message) const
	{
		quint32 classnamehash = VFrame30::CVFrameUtils::GetClassHashCode("FblElement");
		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		::Proto::FblElement* pMutableFblElement = message->mutable_fblelement();

		VFrame30::Proto::Write(pMutableFblElement->mutable_guid(), m_guid);
		VFrame30::Proto::Write(pMutableFblElement->mutable_strid(), m_strID);
		VFrame30::Proto::Write(pMutableFblElement->mutable_caption(), m_caption);
		pMutableFblElement->set_opcode(m_opcode);

		for (auto signal = m_inputSignals.begin(); signal != m_inputSignals.end(); ++signal)
		{
			::Proto::FblElementSignal* s = pMutableFblElement->mutable_inputsignals()->Add();
			signal->SaveData(s);
		}

		for (auto signal = m_outputSignals.begin(); signal != m_outputSignals.end(); ++signal)
		{
			::Proto::FblElementSignal* s = pMutableFblElement->mutable_outputsignals()->Add();
			signal->SaveData(s);
		}

		for (auto param = m_params.begin(); param != m_params.end(); ++param)
		{
			::Proto::FblElementParam* p = pMutableFblElement->mutable_params()->Add();
			param->SaveData(p);
		}
		
		return true;
	}

	bool FblElement::LoadData(const ::Proto::Envelope& message)
	{
		if (message.has_fblelement() == false)
		{
			assert(message.has_fblelement());
			return false;
		}

		const ::Proto::FblElement& fblelement = message.fblelement();

		m_guid = VFrame30::Proto::Read(fblelement.guid());
		m_strID = VFrame30::Proto::Read(fblelement.strid());
		m_caption = VFrame30::Proto::Read(fblelement.caption());
		m_opcode = fblelement.opcode();
			
		// Read input signals
		//
		m_inputSignals.clear();
		for (int i = 0; i < fblelement.inputsignals().size(); i++)
		{
			Fbl::FblElementSignal s;
			s.LoadData(fblelement.inputsignals(i));
			
			m_inputSignals.push_back(s);
		}

		// Read output signals
		//
		m_outputSignals.clear();
		for (int i = 0; i < fblelement.outputsignals().size(); i++)
		{
			Fbl::FblElementSignal s;
			s.LoadData(fblelement.outputsignals(i));
			
			m_outputSignals.push_back(s);
		}

		// Read params
		//
		m_params.clear();
		for (int i = 0; i < fblelement.params().size(); i++)
		{
			Fbl::FblElementParam p;
			p.LoadData(fblelement.params(i));
			
			m_params.push_back(p);
		}

		return true;
	}

	FblElement* FblElement::CreateObject(const ::Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_fblelement() == false)
		{
			assert(message.has_fblelement());
			return nullptr;
		}

		FblElement* pFblElement = new FblElement();
		
		pFblElement->LoadData(message);

		return pFblElement;
	}

	// Properties and Data
	//

	// Guid
	//
	const QUuid& FblElement::guid() const
	{
		return m_guid;
	}
	void FblElement::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// StrID
	//
	const QString& FblElement::strID() const
	{
		return m_strID;
	}
	void FblElement::setStrID(const QString& strID)
	{
		m_strID = strID;
	}

	// Caption
	//
	const QString& FblElement::caption() const
	{
		return m_caption;
	}
	void FblElement::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Opcode
	//
	unsigned int FblElement::opcode() const
	{
		return m_opcode;
	}
	void FblElement::setOpcode(unsigned int value)
	{
		m_opcode = value;
	}

	// InputSignals
	//
	const std::vector<FblElementSignal>& FblElement::inputSignals() const
	{
		return m_inputSignals;
	}
	void FblElement::setInputSignals(const std::vector<FblElementSignal>& inputsignals)
	{
		m_inputSignals = inputsignals;
	}

	// OutputSignals
	//
	const std::vector<FblElementSignal>& FblElement::outputSignals() const
	{
		return m_outputSignals;
	}
	void FblElement::setOutputSignals(const std::vector<FblElementSignal>& outputsignals)
	{
		m_outputSignals = outputsignals;
	}

	// Params
	//
	const std::vector<FblElementParam>& FblElement::params() const
	{
		return m_params;
	}
	void FblElement::setParams(const std::vector<FblElementParam>& params)
	{
		m_params = params;
	}


	//
	//
	//				FblElementCollection - Коллекция прототипов FBL элементов
	//
	//

	FblElementCollection::FblElementCollection(void)
	{
		Init();
	}

	FblElementCollection::~FblElementCollection(void)
	{
	}

	void FblElementCollection::Init(void)
	{
	}

	std::shared_ptr<FblElement> FblElementCollection::Get(const QUuid& guid) const
	{
		auto result = std::find_if(elements.begin(), elements.end(),
			[&guid](const std::shared_ptr<FblElement>& fblelement)
			{
				return fblelement->guid() == guid;
			});
		
		return result == elements.end() ? std::shared_ptr<FblElement>(nullptr) : *result;
	}

}

