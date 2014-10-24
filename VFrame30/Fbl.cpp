#include "Stable.h"
#include "Fbl.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace Afbl
{
	//
	//
	//							FblParamValue		
	//
	//
	AfbParamValue::AfbParamValue()
	{
		IntegralValue = 0;
		FloatingPoint = 0.0;
		Discrete = false;
		Type = AfbParamType::AnalogIntegral;
	}

	AfbParamValue::AfbParamValue(long long value, AfbParamType type)
	{
		Q_ASSERT(type == AfbParamType::AnalogIntegral);

		IntegralValue = value;
		FloatingPoint = 0.0;
		Discrete = false;
		Type = type;
	}

	AfbParamValue::AfbParamValue(double value, AfbParamType type)
	{
		Q_ASSERT(type == AfbParamType::AnalogFloatingPoint);

		IntegralValue = 0;
		FloatingPoint = value;
		Discrete = false;
		Type = type;
	}

	AfbParamValue::AfbParamValue(bool value, AfbParamType type)
	{
		Q_ASSERT(type == AfbParamType::DiscreteValue);

		IntegralValue = 0;
		FloatingPoint = 0.0;
		Discrete = value;
		Type = type;
	}


	bool AfbParamValue::SaveData(Proto::FblParamValue* message) const
	{
		message->set_integralvalue(IntegralValue);
		message->set_floatingpoint(FloatingPoint);
		message->set_discrete(Discrete);
		return true;
	}

	bool AfbParamValue::LoadData(const Proto::FblParamValue& message)
	{
		IntegralValue = message.integralvalue();
		FloatingPoint = message.floatingpoint();
		Discrete = message.discrete();
		return true;
	}

	bool AfbParamValue::loadFromXml(QXmlStreamReader* xmlReader)
	{
		if (xmlReader == nullptr)
		{
			Q_ASSERT(xmlReader);
			return false;
		}

		if (xmlReader->name() != "AfbParamValue")
		{
			xmlReader->raiseError(QObject::tr("AfbParamValue expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("Type"))
		{
			QString stringType = xmlReader->attributes().value("Type").toString();
			if (stringType == "AnalogIntegral")
			{
				Type = AfbParamType::AnalogIntegral;
			}
			else
			{
				if (stringType == "AnalogFloatingPoint")
				{
					Type = AfbParamType::AnalogFloatingPoint;
				}
				else
				{
					if (stringType == "DiscreteValue")
					{
						Type = AfbParamType::DiscreteValue;
					}
					else
					{
						Q_ASSERT(false);
					}
				}
			}
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbParamValue - No Type found"));
		}

		if (xmlReader->attributes().hasAttribute("Value"))
		{
			switch (Type)
			{
				case AfbParamType::AnalogIntegral:
					{
						IntegralValue = xmlReader->attributes().value("Value").toLongLong();
						//qDebug()<<"Param Value = "<< QString::number(IntegralValue);
					}
					break;
				case AfbParamType::AnalogFloatingPoint:
					{
						FloatingPoint = xmlReader->attributes().value("Value").toDouble();
						//qDebug()<<"Param Value = "<< QString::number(FloatingPoint);
					}
					break;
				case AfbParamType::DiscreteValue:
					{
						Discrete = xmlReader->attributes().value("Value") == "0" ? false : true;
						//qDebug()<<"Param Value = "<< QString(Discrete ? "true" : "false");
					}
					break;
				default:
					Q_ASSERT(false);
			}
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbParamValue - No Value found"));
		}

		//qDebug()<<"Param Type = "<< QString::number(Type);

		QXmlStreamReader::TokenType endToken = xmlReader->readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return !xmlReader->hasError();

	}

	bool AfbParamValue::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->writeStartElement("AfbParamValue");

		switch (Type)
		{
			case AnalogIntegral:
				{
					xmlWriter->writeAttribute("Type", "AnalogIntegral");
					xmlWriter->writeAttribute("Value", QString::number(IntegralValue));
				}
				break;
			case AnalogFloatingPoint:
				{
					xmlWriter->writeAttribute("Type", "AnalogFloatingPoint");
					xmlWriter->writeAttribute("Value", QString::number(FloatingPoint));
				}
				break;
			case DiscreteValue:
				{
					xmlWriter->writeAttribute("Type", "DiscreteValue");
					xmlWriter->writeAttribute("Value", QString::number(Discrete));
				}
				break;
			default:
				Q_ASSERT(false);
		}

		xmlWriter->writeEndElement();

		return true;
	}

	QVariant AfbParamValue::toQVariant() const
	{
		switch(Type)
		{
			case AfbParamType::AnalogIntegral:
				return QVariant(IntegralValue);
			case AfbParamType::AnalogFloatingPoint:
				return QVariant(FloatingPoint);
			case AfbParamType::DiscreteValue:
				return QVariant(Discrete);
			default:
				assert(false);
				return QVariant();
		}
	}

	AfbParamValue AfbParamValue::fromQVariant(QVariant value)
	{
		switch (value.type())
		{
			case QMetaType::Int:
			case QMetaType::UInt:
			case QMetaType::Long:
			case QMetaType::LongLong:
				return AfbParamValue(static_cast<long long>(value.toLongLong()), AfbParamType::AnalogIntegral);
			case QMetaType::Double:
				return AfbParamValue(value.toDouble(), AfbParamType::AnalogFloatingPoint);
			case QMetaType::Bool:
				return AfbParamValue(value.toBool(), AfbParamType::DiscreteValue);
			default:
				assert(false);
				return AfbParamValue();
		}
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

	bool AfbElementSignal::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->writeStartElement("AfbElementSignal");
		xmlWriter->writeAttribute("Caption", caption());
		xmlWriter->writeAttribute("Type", type() == AfbSignalType::Analog ? "Analog" : "Discrete");
		xmlWriter->writeEndElement();

		return true;
	}

	bool AfbElementSignal::loadFromXml(QXmlStreamReader* xmlReader)
	{
		if (xmlReader == nullptr)
		{
			Q_ASSERT(xmlReader);
			return false;
		}

		if (xmlReader->name() != "AfbElementSignal")
		{
			xmlReader->raiseError(QObject::tr("AfbElementSignal expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("Caption"))
		{
			setCaption(xmlReader->attributes().value("Caption").toString());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElementSignal - No Caption found"));
		}

		if (xmlReader->attributes().hasAttribute("Type"))
		{
			if (xmlReader->attributes().value("Type").toString() == "Analog")
			{
				setType(AfbSignalType::Analog);
			}
			else
			{
				setType(AfbSignalType::Discrete);
			}

		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElementSignal - No Type found"));
		}

		//qDebug()<<"Signal caption = "<< caption();
		//qDebug()<<"Signal Type = "<< QString::number(type());

		QXmlStreamReader::TokenType endToken = xmlReader->readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return !xmlReader->hasError();

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

	void AfbElementParam::update(const AfbParamType& type, const AfbParamValue& lowLimit, const AfbParamValue& highLimit)
	{
		assert(type == lowLimit.Type && type == highLimit.Type);

		m_type = type;
		m_lowLimit = lowLimit;
		m_highLimit = highLimit;

		return;
	}

	bool AfbElementParam::SaveData(Proto::FblElementParam* message) const
	{
		Proto::Write(message->mutable_caption(), m_caption);
		message->set_type(static_cast<Proto::FblParamType>(m_type));

		m_value.SaveData(message->mutable_value());
		m_defaultValue.SaveData(message->mutable_defaultvalue());

		m_lowLimit.SaveData(message->mutable_lowlimit());
		m_highLimit.SaveData(message->mutable_highlimit());

		return true;
	}

	bool AfbElementParam::LoadData(const Proto::FblElementParam& message)
	{
		m_caption = Proto::Read(message.caption());
		m_type = static_cast<AfbParamType>(message.type());

		m_value.LoadData(message.value());
		m_defaultValue.LoadData(message.defaultvalue());

		m_lowLimit.LoadData(message.lowlimit());
		m_highLimit.LoadData(message.highlimit());

		return true;
	}

	bool AfbElementParam::loadFromXml(QXmlStreamReader* xmlReader)
	{
		if (xmlReader == nullptr)
		{
			Q_ASSERT(xmlReader);
			return false;
		}

		if (xmlReader->name() != "AfbElementParam")
		{
			xmlReader->raiseError(QObject::tr("AfbElementParam expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("Caption"))
		{
			setCaption(xmlReader->attributes().value("Caption").toString());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElementParam - No Caption found"));
		}

		if (xmlReader->attributes().hasAttribute("Type"))
		{
			setType((AfbParamType)xmlReader->attributes().value("Type").toInt());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElementParam - No Type found"));
		}

		//qDebug()<<"Param caption = "<< caption();
		//qDebug()<<"Param Type = "<< QString::number(type());

		// Read values
		//
		while (xmlReader->readNextStartElement())
		{
			//qDebug()<<xmlReader->name();

			QString valueName = xmlReader->name().toString();

			while (xmlReader->readNextStartElement())
			{
				AfbParamValue value;
				value.loadFromXml(xmlReader);

				if (valueName == "Value")
				{
					setValue(value);
				}
				if (valueName == "Default")
				{
					setDefaultValue(value);
				}
				if (valueName == "LowLimit")
				{
					setLowLimit(value);
				}
				if (valueName == "HighLimit")
				{
					setHighLimit(value);
				}
			}
		}

		return !xmlReader->hasError();

	}

	bool AfbElementParam::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->writeStartElement("AfbElementParam");
		xmlWriter->writeAttribute("Caption", caption());

		switch (type())
		{
			case AnalogIntegral:
				{
					xmlWriter->writeAttribute("Type", "AnalogIntegral");
				}
				break;
			case AnalogFloatingPoint:
				{
					xmlWriter->writeAttribute("Type", "AnalogFloatingPoint");
				}
				break;
			case DiscreteValue:
				{
					xmlWriter->writeAttribute("Type", "DiscreteValue");
				}
				break;
			default:
				Q_ASSERT(false);
		}

		xmlWriter->writeStartElement("Value");
		value().saveToXml(xmlWriter);
		xmlWriter->writeEndElement();

		xmlWriter->writeStartElement("Default");
		defaultValue().saveToXml(xmlWriter);
		xmlWriter->writeEndElement();

		xmlWriter->writeStartElement("LowLimit");
		lowLimit().saveToXml(xmlWriter);
		xmlWriter->writeEndElement();

		xmlWriter->writeStartElement("HighLimit");
		highLimit().saveToXml(xmlWriter);
		xmlWriter->writeEndElement();

		xmlWriter->writeEndElement();

		return true;
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
		m_guid = QUuid::createUuid();
	}

	bool AfbElement::loadFromXml(const Proto::AfbElementXml& data)
	{
		QByteArray ba(data.data().data(), static_cast<int>(data.data().size()));
		bool result = loadFromXml(ba);
		return result;
	}

	bool AfbElement::loadFromXml(const QByteArray& data)
	{
		QXmlStreamReader reader(data);
		bool result = loadFromXml(&reader);
		return result;
	}

	bool AfbElement::loadFromXml(QXmlStreamReader* xmlReader)
	{
		if (xmlReader == nullptr)
		{
			Q_ASSERT(xmlReader);
			return false;
		}

		if (xmlReader->readNextStartElement() == false)
		{
			return !xmlReader->hasError();
		}

		//qDebug()<<xmlReader->name();
		if (xmlReader->name() != "ApplicationFunctionalBlocks")
		{
			xmlReader->raiseError(QObject::tr("The file is not an ApplicationFunctionalBlocks file."));
			return !xmlReader->hasError();
		}

		if (xmlReader->readNextStartElement() == false)
		{
			return !xmlReader->hasError();
		}


		if (xmlReader->name() != "AfbElement")
		{
			xmlReader->raiseError(QObject::tr("AfbElement expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("Guid"))

		{
			setGuid(QUuid(xmlReader->attributes().value("Guid").toString()));
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElement - No Guid found"));
		}

		if (xmlReader->attributes().hasAttribute("StrId"))
		{
			setStrID(xmlReader->attributes().value("StrId").toString());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElement - No StrId found"));
		}

		if (xmlReader->attributes().hasAttribute("Caption"))
		{
			setCaption(xmlReader->attributes().value("Caption").toString());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElement - No Caption found"));
		}

		if (xmlReader->attributes().hasAttribute("OpCode"))
		{
			setOpcode(xmlReader->attributes().value("OpCode").toInt());
		}
		else
		{
			xmlReader->raiseError(QObject::tr("AfbElement - No OpCode found"));
		}

		//qDebug()<<"Guid ="<<guid().toString();
		//qDebug()<<"StrId ="<<strID();
		//qDebug()<<"Caption ="<<caption();
		//qDebug()<<"OpCode ="<<opcode();

		std::vector<AfbElementSignal> inputSignals;
		std::vector<AfbElementSignal> outputSignals;
		std::vector<AfbElementParam> params;

		while (xmlReader->readNextStartElement())
		{
			// Input or output signals
			//
			if (xmlReader->name() == "OutputSignals" || xmlReader->name() == "InputSignals")
			{
				enum LoadSignalType {InputSignal, OutputSignal} loadSignalType;
				loadSignalType = xmlReader->name() == "OutputSignals" ? LoadSignalType::OutputSignal
															: LoadSignalType::InputSignal;

				//qDebug()<<xmlReader->name().toString();

				// Read signals
				//
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "AfbElementSignal")
					{
						//qDebug()<<xmlReader->name().toString();

						AfbElementSignal afbSignal;
						afbSignal.loadFromXml(xmlReader);
						if (loadSignalType == LoadSignalType::InputSignal)
						{
							inputSignals.push_back(afbSignal);
						}
						else
						{
							outputSignals.push_back(afbSignal);
						}
					}
					else
					{
						xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
						xmlReader->skipCurrentElement();
					}
				}
				continue;
			}

			if (xmlReader->name() == "Params")
			{
				//qDebug()<<xmlReader->name().toString();

				// Read params
				//
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "AfbElementParam")
					{
						//qDebug()<<xmlReader->name().toString();

						AfbElementParam afbParam;
						afbParam.loadFromXml(xmlReader);
						params.push_back(afbParam);
					}
					else
					{
						xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
						xmlReader->skipCurrentElement();
					}
				}
				continue;
			}

			xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
			xmlReader->skipCurrentElement();
		}

		//qDebug()<<"finished.";

		setInputSignals(inputSignals);
		setOutputSignals(outputSignals);
		setParams(params);

		return !xmlReader->error();

	}

	bool AfbElement::saveToXml(Proto::AfbElementXml* dst) const
	{
		QByteArray ba;

		QXmlStreamWriter writer(&ba);
		bool result = saveToXml(&writer);

		if (result == true)
		{
			dst->set_data(ba.data(), ba.size());
		}

		return result;
	}

	bool AfbElement::saveToXml(QByteArray* dst) const
	{
		QXmlStreamWriter writer(dst);
		bool result = saveToXml(&writer);
		return result;
	}

	bool AfbElement::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->setAutoFormatting(true);
		xmlWriter->writeStartDocument();

		xmlWriter->writeStartElement("ApplicationFunctionalBlocks");

		xmlWriter->writeStartElement("AfbElement");
		xmlWriter->writeAttribute("Guid", guid().toString());
		xmlWriter->writeAttribute("StrId", strID());
		xmlWriter->writeAttribute("Caption", caption());
		xmlWriter->writeAttribute("OpCode", QString::number(opcode()));

		xmlWriter->writeStartElement("InputSignals");
		for (auto s : inputSignals())
		{
			s.saveToXml(xmlWriter);
		}
		xmlWriter->writeEndElement(); //"InputSignals"

		xmlWriter->writeStartElement("OutputSignals");
		for (auto s : outputSignals())
		{
			s.saveToXml(xmlWriter);
		}
		xmlWriter->writeEndElement(); //"OutputSignals"

		xmlWriter->writeStartElement("Params");
		for (auto s : params())
		{
			s.saveToXml(xmlWriter);
		}
		xmlWriter->writeEndElement(); //"Params"

		xmlWriter->writeEndElement(); //"AfbElement"

		xmlWriter->writeEndElement(); //"ApplicationFunctionalBlocks"

		xmlWriter->writeEndDocument();


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

	void AfbElement::updateParams(const std::vector<AfbElementParam>& params)
	{
		std::vector<AfbElementParam> newParams;
		newParams.reserve(params.size());

		for (const AfbElementParam& p : params)
		{
			std::vector<AfbElementParam>::iterator found = std::find_if(m_params.begin(), m_params.end(),
				[&p](const AfbElementParam& mp)
				{
					return p.caption() == mp.caption() && p.type() == mp.type();
				});

			if (found != m_params.end())
			{
				found->update(p.type(), p.lowLimit(), p.highLimit());
				newParams.push_back(*found);
			}
			else
			{
				newParams.push_back(p);
			}
		}
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

	bool AfbElementCollection::SaveData(Proto::AfbElementCollection* message) const
	{
		for (const std::shared_ptr<AfbElement>& e : m_elements)
		{
			e->saveToXml(message->add_elements());
		}

		return true;
	}

	bool AfbElementCollection::LoadData(const Proto::AfbElementCollection& message)
	{
		m_elements.clear();
		m_elements.reserve(message.elements_size());

		for (int i = 0; i < message.elements_size(); i++)
		{
			std::shared_ptr<Afbl::AfbElement> e = std::make_shared<Afbl::AfbElement>();
			e->loadFromXml(message.elements(i));

			m_elements.push_back(e);
		}

		return true;
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

