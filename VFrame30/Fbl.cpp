#include "Stable.h"
#include "Fbl.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

namespace Afbl
{
	//
	//
	//							CFblElementSignal		
	//
	//
	AfbElementSignal::AfbElementSignal(void):
		m_type(AfbSignalType::Analog),
		m_operandIndex(0),
		m_size(0)
	{
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

        //message->set_index(index());
        //message->set_size(size());
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
		xmlWriter->writeAttribute("OpIndex", QString::number(operandIndex()));
        xmlWriter->writeAttribute("Size", QString::number(size()));
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

		if (xmlReader->attributes().hasAttribute("OpIndex"))
        {
			setOperandIndex(xmlReader->attributes().value("OpIndex").toInt());
        }

        if (xmlReader->attributes().hasAttribute("Size"))
        {
            setSize(xmlReader->attributes().value("Size").toInt());
        }

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

    int AfbElementSignal::operandIndex() const
    {
        return m_operandIndex;
    }

    void AfbElementSignal::setOperandIndex(int value)
    {
        m_operandIndex = value;
    }

    int AfbElementSignal::size() const
    {
        return m_size;
    }

    void AfbElementSignal::setSize(int value)
    {
        m_size = value;
    }


    //
	//
	//							CFblElementParam		
	//
	//

	AfbElementParam::AfbElementParam(void):
        m_visible(true),
		m_type(AfbParamType::AnalogIntegral),
		m_operandIndex(0),
		m_size(0),
		m_instantiator(false),
		m_user(false)

    {
		m_value = 0;
		m_defaultValue = 0;
		m_lowLimit = 0;
		m_highLimit = 0;
	}

	AfbElementParam::~AfbElementParam(void)
	{
	}

	void AfbElementParam::update(const AfbParamType& type, const QVariant &lowLimit, const QVariant &highLimit)
	{

		m_type = type;
		m_lowLimit = lowLimit;
		m_highLimit = highLimit;

		return;
	}

	bool AfbElementParam::SaveData(Proto::FblElementParam* message) const
	{
		Proto::Write(message->mutable_caption(), m_caption);
		message->set_visible(visible());
		message->set_type(static_cast<Proto::FblParamType>(m_type));

		Proto::Write(message->mutable_value(), m_value);
		Proto::Write(message->mutable_defaultvalue(), m_defaultValue);
		Proto::Write(message->mutable_lowlimit(), m_lowLimit);
		Proto::Write(message->mutable_highlimit(), m_highLimit);

		message->set_operandindex(operandIndex());
        message->set_size(size());
		message->set_instantiator(instantiator());
		message->set_user(user());
		Proto::Write(message->mutable_changedscript(), m_changedScript);

		return true;
	}

	bool AfbElementParam::LoadData(const Proto::FblElementParam& message)
	{
		Proto::Read(message.caption(), &m_caption);
		m_type = static_cast<AfbParamType>(message.type());
		m_visible = message.visible();

		m_value = Proto::Read(message.value());
		m_defaultValue = Proto::Read(message.defaultvalue());
		m_lowLimit = Proto::Read(message.lowlimit());
		m_highLimit = Proto::Read(message.highlimit());

		m_operandIndex = message.operandindex();
		m_size = message.size();
		m_instantiator = message.instantiator();
		m_user = message.user();
		Proto::Read(message.changedscript(), &m_changedScript);

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

		if (xmlReader->attributes().hasAttribute("Visible"))
		{
			setVisible(xmlReader->attributes().value("Visible").toString() == "true" ? true : false);
		}

		if (xmlReader->attributes().hasAttribute("Type"))
		{
			setType((AfbParamType)xmlReader->attributes().value("Type").toInt());
		}

		if (xmlReader->attributes().hasAttribute("OpIndex"))
        {
			setOperandIndex(xmlReader->attributes().value("OpIndex").toInt());
        }

        if (xmlReader->attributes().hasAttribute("Size"))
        {
            setSize(xmlReader->attributes().value("Size").toInt());
        }

		if (xmlReader->attributes().hasAttribute("Instantiator"))
		{
			setInstantiator(xmlReader->attributes().value("Instantiator").toString() == "true" ? true : false);
		}

		if (xmlReader->attributes().hasAttribute("User"))
		{
			setUser(xmlReader->attributes().value("User").toString() == "true" ? true : false);
		}

		// Read values
		//
		while (xmlReader->readNextStartElement())
		{
			QString valueName = xmlReader->name().toString();

			if (valueName == "Script")
			{
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "Changed")
					{
						setChangedScript(xmlReader->readElementText());
					}
					else
					{
						xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
					}
				}
			}


			if (valueName == "Value" || valueName == "Default" || valueName == "LowLimit" || valueName == "HighLimit")
			{
				QString str = xmlReader->readElementText();
				QVariant val;

				switch (type())
				{
					case AnalogIntegral:
					{
						val = str.toInt();
						break;
					}
					case AnalogFloatingPoint:
					{
						val = str.toDouble();
						break;
					}
					case DiscreteValue:
					{
						val = str == "1" ? true : false;
						break;
					}
				default:
					assert(false);
				}

				if (val.isNull())
				{
					xmlReader->raiseError(QObject::tr("AfbElementParam, unknown type, tag ") + valueName + QObject::tr(", data ") + str);
				}
				else
				{
					if (valueName == "Value")
					{
						setValue(val);
					}
					if (valueName == "Default")
					{
						setDefaultValue(val);
					}
					if (valueName == "LowLimit")
					{
						setLowLimit(val);
					}
					if (valueName == "HighLimit")
					{
						setHighLimit(val);
					}
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
		xmlWriter->writeAttribute("Visible", visible() ? "true" : "false");
		xmlWriter->writeAttribute("OpIndex", QString::number(operandIndex()));
        xmlWriter->writeAttribute("Size", QString::number(size()));
		xmlWriter->writeAttribute("Instantiator", instantiator() ? "true" : "false");
		xmlWriter->writeAttribute("User", user() ? "true" : "false");

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

		if (type() == DiscreteValue)
		{
			xmlWriter->writeTextElement("Value", value() == true ? "1" : "0");
			xmlWriter->writeTextElement("Default", defaultValue() == true ? "1" : "0");
			xmlWriter->writeTextElement("LowLimit", lowLimit() == true ? "1" : "0");
			xmlWriter->writeTextElement("HighLimit", highLimit() == true ? "1" : "0");
		}
		else
		{
			xmlWriter->writeTextElement("Value", value().toString());
			xmlWriter->writeTextElement("Default", defaultValue().toString());
			xmlWriter->writeTextElement("LowLimit", lowLimit().toString());
			xmlWriter->writeTextElement("HighLimit", highLimit().toString());
		}

		xmlWriter->writeStartElement("Script");
		xmlWriter->writeTextElement("Changed", changedScript());
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

	bool AfbElementParam::visible() const
	{
		return m_visible;
	}

	void AfbElementParam::setVisible(bool visible)
	{
		m_visible = visible;
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
	const QVariant& AfbElementParam::value() const
	{
		return m_value;
	}
	void AfbElementParam::setValue(const QVariant& value)
	{
		m_value = value;
	}

	// Defaut Value
	//
	const QVariant& AfbElementParam::defaultValue() const
	{
		return m_defaultValue;
	}
	void AfbElementParam::setDefaultValue(const QVariant& defaultValue)
	{
		m_defaultValue = defaultValue;
	}

	// LowLimit
	const QVariant& AfbElementParam::lowLimit() const
	{
		return m_lowLimit;
	}
	void AfbElementParam::setLowLimit(const QVariant& lowLimit)
	{
		m_lowLimit = lowLimit;
	}

	// highLimit
	//
	const QVariant& AfbElementParam::highLimit() const
	{
		return m_highLimit;
	}
	void AfbElementParam::setHighLimit(const QVariant& highLimit)
	{
		m_highLimit = highLimit;
	}

	int AfbElementParam::operandIndex() const
    {
		return m_operandIndex;
    }

	void AfbElementParam::setOperandIndex(int value)
    {
		m_operandIndex = value;
    }

    int AfbElementParam::size() const
    {
        return m_size;
    }

    void AfbElementParam::setSize(int value)
    {
        m_size = value;
    }

	bool AfbElementParam::instantiator() const
	{
		return m_instantiator;
	}

	void AfbElementParam::setInstantiator(bool value)
	{
		m_instantiator = value;
	}

	bool AfbElementParam::user() const
	{
		return m_user;
	}

	void AfbElementParam::setUser(bool value)
	{
		m_user = value;
	}

	QString AfbElementParam::changedScript() const
	{
		return m_changedScript;
	}

	void AfbElementParam::setChangedScript(const QString &value)
	{
		m_changedScript = value;
	}

    //
	//
	//							FblElement		
	//
	//

	AfbElement::AfbElement(void):
		m_opcode(0),
		m_hasRam(false)
	{
	}

	AfbElement::~AfbElement(void)
	{
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

		if (xmlReader->attributes().hasAttribute("StrId"))
		{
			setStrID(xmlReader->attributes().value("StrId").toString());
		}

		std::vector<AfbElementSignal> inputSignals;
		std::vector<AfbElementSignal> outputSignals;
		std::vector<AfbElementParam> params;

		while (xmlReader->readNextStartElement())
		{
			if (xmlReader->name() == "Properties")
			{
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "Caption")
					{
						setCaption(xmlReader->readElementText());
					}

					if (xmlReader->name() == "OpCode")
					{
						setOpcode(xmlReader->readElementText().toInt());
					}

					if (xmlReader->name() == "hasRam")
					{
						setHasRam(xmlReader->readElementText() == "true" ? true : false);
					}
				}

				continue;
			}

			// Input or output signals
			//
			if (xmlReader->name() == "OutputSignals" || xmlReader->name() == "InputSignals")
			{
				enum LoadSignalType {InputSignal, OutputSignal} loadSignalType;
				loadSignalType = xmlReader->name() == "OutputSignals" ? LoadSignalType::OutputSignal
															: LoadSignalType::InputSignal;

				// Read signals
				//
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "AfbElementSignal")
					{
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
				// Read params
				//
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "AfbElementParam")
					{
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

			if (xmlReader->name() == "CommonScript")
			{
				// Read params
				//
				while (xmlReader->readNextStartElement())
				{
					if (xmlReader->name() == "Library")
					{
						setLibraryScript(xmlReader->readElementText());
					}
					else
					{
						if (xmlReader->name() == "AfterCreation")
						{
							setAfterCreationScript(xmlReader->readElementText());
						}
						else
						{
							xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
						}
					}
				}
				continue;
			}

			xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
			xmlReader->skipCurrentElement();
		}

		setInputSignals(inputSignals);
		setOutputSignals(outputSignals);
		setParams(params);

		return !xmlReader->hasError();

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
		xmlWriter->writeAttribute("StrId", strID());


		xmlWriter->writeStartElement("Properties");
		xmlWriter->writeTextElement("Caption", caption());
		xmlWriter->writeTextElement("OpCode", QString::number(opcode()));
		xmlWriter->writeTextElement("hasRam", hasRam() ? "true" : "false");
		xmlWriter->writeEndElement();

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

		xmlWriter->writeStartElement("CommonScript");

		xmlWriter->writeTextElement("Library", libraryScript());
		xmlWriter->writeTextElement("AfterCreation", afterCreationScript());

		xmlWriter->writeEndElement(); //"CommonScript"

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
//		message->set_classnamehash(classnamehash);	// ������������ ����, �� ����� ������, �� ���� ����������������� �����.

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

//		// ��� ������� ����� ��������� ������ ���� ���������
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
	QString AfbElement::caption() const
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

	bool AfbElement::hasRam() const
	{
		return m_hasRam;
	}

	void AfbElement::setHasRam(bool value)
	{
		m_hasRam = value;
	}

	QString AfbElement::libraryScript() const
	{
		return m_libraryScript;
	}

	void AfbElement::setLibraryScript(const QString& value)
	{
		m_libraryScript = value;
	}

	QString AfbElement::afterCreationScript() const
	{
		return m_afterCreationScript;
	}

	void AfbElement::setAfterCreationScript(const QString& value)
	{
		m_afterCreationScript = value;
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

	std::vector<AfbElementParam>& AfbElement::params()
	{
		return m_params;
	}

	int AfbElement::paramsCount() const
	{
		return static_cast<int>(m_params.size());
	}

	void AfbElement::setParams(const std::vector<AfbElementParam>& params)
	{
		m_params = params;
	}

	//
	//
	//				FblElementCollection - ��������� ���������� FBL ���������
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

	std::shared_ptr<AfbElement> AfbElementCollection::get(const QString& strID) const
	{
		auto result = std::find_if(m_elements.begin(), m_elements.end(),
			[&strID](const std::shared_ptr<AfbElement>& fblelement)
			{
				return fblelement->strID() == strID;
			});
		
		return result == m_elements.end() ? std::shared_ptr<AfbElement>() : *result;
	}

}

