#include "Stable.h"
#include "Afb.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

bool operator== (const Afb::AfbType& t1, const Afb::AfbType& t2)
{
	return t1.m_type == t2.m_type;
}

bool operator!= (const Afb::AfbType& t1, const Afb::AfbType& t2)
{
	return t1.m_type != t2.m_type;
}


bool operator< (const Afb::AfbType& t1, const Afb::AfbType& t2)
{
	return t1.m_type < t2.m_type;
}

namespace Afb
{
	AfbType::AfbType() :
		m_type(Type::UNKNOWN)
	{
	}

	AfbType::AfbType(const AfbType& t) :
		m_type(t.m_type)
	{
	}

	AfbType::AfbType(AfbType::Type t) :
		m_type(t)
	{
	}

	void AfbType::fromOpCode(int opCode)
	{
#ifdef Q_DEBUG
		if (toText(opCode) == "UNKNOWN")
		{
			assert(false);
			m_type = Type::UNKNOWN;
			return;
		}
#endif
		m_type = static_cast<Type>(opCode);
		return;
	}

	int AfbType::toOpCode() const
	{
		return static_cast<int>(m_type);
	}

	QString AfbType::text() const
	{
		return toText(toOpCode());
	}

	QString AfbType::toText() const
	{
		return toText(toOpCode());
	}

	QString AfbType::toText(int opCode)
	{
		switch (static_cast<Type>(opCode))
		{
			case Type::UNKNOWN:
				return "UNKNOWN";
			case Type::AND:
				return "AND";
			case Type::OR:
				return "OR";
			case Type::XOR:
				return "XOR";
			case Type::NOT:
				return "NOT";
			case Type::TCT:
				return "TCT";
			case Type::SR_RS:
				return "SR_RS";
			case Type::CTUD:
				return "CTUD";
			case Type::MAJ:
				return "MAJ";
			case Type::SRSST:
				return "SRSST";
			case Type::BCOD:
				return "BCOD";
			case Type::BDEC:
				return "BDEC";
			case Type::BCOMP:
				return "BCOMP";
			case Type::LAG:
				return "LAG";
			case Type::MID:
				return "MID";
			case Type::ADD:
				return "ADD";
			case Type::SCAL:
				return "SCAL";
			case Type::LINFUN:
				return "LINFUN";
			case Type::SQRT:
				return "SQRT";
			case Type::SIN:
				return "SIN";
			case Type::COS:
				return "COS";
			case Type::DIV:
				return "DIV";
			case Type::MULT:
				return "MULT";
			case Type::ABS:
				return "ABS";
			case Type::LN:
				return "LN";
			case Type::LIM:
				return "LIM";
			case Type::MIN_MAX:
				return "MIN_MAX";
			case Type::PID:
				return "PID";
			default:
				assert(false);
				return "UNKNOWN";
		}
	}


	//
	//							AfbSignal
	//
	AfbSignal::AfbSignal(void):
		m_type(AfbSignalType::Analog),
		m_operandIndex(0),
		m_size(0)
	{
	}

	AfbSignal::~AfbSignal(void)
	{
	}

	AfbSignal::AfbSignal(const AfbSignal& that) : QObject()
	{
		*this = that;
	}

	AfbSignal& AfbSignal::operator=(const AfbSignal& that)
	{
		if (this == &that)
		{
			return *this;
		}

		m_opName = that.m_opName;
		m_caption = that.m_caption;
		m_type = that.m_type;
		m_operandIndex = that.m_operandIndex;
		m_size = that.m_size;

		return *this;
	}

// Serialization
	//
	bool AfbSignal::SaveData(Proto::AfbSignal* /*message*/) const
	{
		assert(false);
		return false;

        //message->set_index(index());
        //message->set_size(size());
//		Proto::Write(message->mutable_caption(), m_caption);
//		message->set_type(static_cast<Proto::FblSignalType>(m_type));
//		return true;
	}
	
	bool AfbSignal::LoadData(const Proto::AfbSignal& /*message*/)
	{
		assert(false);
		return false;

//		m_caption = Proto::Read(message.caption());
//		m_type = static_cast<FblSignalType>(message.type());
//		return true;
	}

	bool AfbSignal::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->writeStartElement("AfbElementSignal");
		xmlWriter->writeAttribute("OpName", opName());
		xmlWriter->writeAttribute("Caption", caption());
		xmlWriter->writeAttribute("Type", type() == AfbSignalType::Analog ? "Analog" : "Discrete");
		xmlWriter->writeAttribute("OpIndex", QString::number(operandIndex()));
        xmlWriter->writeAttribute("Size", QString::number(size()));
		xmlWriter->writeEndElement();

		return true;
	}

	bool AfbSignal::loadFromXml(QXmlStreamReader* xmlReader)
	{
		if (xmlReader == nullptr)
		{
			Q_ASSERT(xmlReader);
			return false;
		}

		if (QString::compare(xmlReader->name().toString(), "AfbElementSignal", Qt::CaseInsensitive) != 0)
		{
			xmlReader->raiseError(QObject::tr("AfbElementSignal expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("OpName"))
		{
			setOpName(xmlReader->attributes().value("OpName").toString());
		}

		if (xmlReader->attributes().hasAttribute("Caption"))
		{
			setCaption(xmlReader->attributes().value("Caption").toString());
		}

		if (xmlReader->attributes().hasAttribute("Type"))
		{
			if (QString::compare(xmlReader->attributes().value("Type").toString(), "Analog", Qt::CaseInsensitive) == 0)
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

	const QString& AfbSignal::opName() const
	{
		return m_opName;

	}

	void AfbSignal::setOpName(const QString& value)
	{
		m_opName = value;
	}

	// Caption
	//
	QString AfbSignal::jsCaption()
	{
		return caption();
	}
	const QString& AfbSignal::caption() const
	{
		return m_caption;
	}
	void AfbSignal::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	AfbSignalType AfbSignal::type() const
	{
		return m_type;
	}
	int AfbSignal::jsType() const
	{
		return static_cast<int>(m_type);
	}
	void AfbSignal::setType(AfbSignalType type)
	{
		m_type = type;
	}

	int AfbSignal::operandIndex() const
    {
        return m_operandIndex;
    }

	void AfbSignal::setOperandIndex(int value)
    {
        m_operandIndex = value;
    }

	int AfbSignal::size() const
    {
        return m_size;
    }

	void AfbSignal::setSize(int value)
    {
        m_size = value;
    }

	bool AfbSignal::isAnalog() const
	{
		return m_type == AfbSignalType::Analog;
	}

	bool AfbSignal::isDiscrete() const
	{
		return m_type == AfbSignalType::Discrete;
	}

    //
	//
	//							AfbParam
	//
	//

	AfbParam::AfbParam(void):
        m_visible(true),
		m_type(AfbParamType::AnalogIntegral),
		m_instantiator(false),
		m_user(false),
		m_operandIndex(0),
		m_size(0)

    {
		m_value = 0;
		m_defaultValue = 0;
		m_lowLimit = 0;
		m_highLimit = 0;
	}

	AfbParam::~AfbParam(void)
	{
	}

	void AfbParam::update(const AfbParamType& type, const QVariant &lowLimit, const QVariant &highLimit)
	{

		m_type = type;
		m_lowLimit = lowLimit;
		m_highLimit = highLimit;

		return;
	}

	bool AfbParam::SaveData(Proto::AfbParam* message) const
	{
		Proto::Write(message->mutable_opname(), m_opName);
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

	bool AfbParam::LoadData(const Proto::AfbParam& message)
	{
		Proto::Read(message.opname(), &m_opName);
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

	bool AfbParam::loadFromXml(QXmlStreamReader* xmlReader)
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

		if (xmlReader->attributes().hasAttribute("OpName"))
		{
			setOpName(xmlReader->attributes().value("OpName").toString());
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

			if (QString::compare(valueName, "Script", Qt::CaseInsensitive) == 0)
			{
				while (xmlReader->readNextStartElement())
				{
					if (QString::compare(xmlReader->name().toString(), "Changed", Qt::CaseInsensitive) == 0)
					{
						setChangedScript(xmlReader->readElementText());
					}
					else
					{
						xmlReader->raiseError(QObject::tr("Unknown tag: ") + xmlReader->name().toString());
					}
				}
			}


			if (QString::compare(valueName, "Value", Qt::CaseInsensitive) == 0
					|| QString::compare(valueName, "Default", Qt::CaseInsensitive) == 0
					|| QString::compare(valueName, "LowLimit", Qt::CaseInsensitive) == 0
					|| QString::compare(valueName, "HighLimit", Qt::CaseInsensitive) == 0)
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
					if (QString::compare(valueName, "Value", Qt::CaseInsensitive) == 0)
					{
						setValue(val);
					}
					if (QString::compare(valueName, "Default", Qt::CaseInsensitive) == 0)
					{
						setDefaultValue(val);
					}
					if (QString::compare(valueName, "LowLimit", Qt::CaseInsensitive) == 0)
					{
						setLowLimit(val);
					}
					if (QString::compare(valueName, "HighLimit", Qt::CaseInsensitive) == 0)
					{
						setHighLimit(val);
					}
				}
			}
		}

		return !xmlReader->hasError();

	}

	bool AfbParam::saveToXml(QXmlStreamWriter* xmlWriter) const
	{
		if (xmlWriter == nullptr)
		{
			Q_ASSERT(xmlWriter);
			return false;
		}

		xmlWriter->writeStartElement("AfbElementParam");
		xmlWriter->writeAttribute("OpName", opName());
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
	const QString& AfbParam::caption() const
	{
		return m_caption;
	}
	void AfbParam::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	const QString& AfbParam::opName() const
	{
		return m_opName;
	}

	void AfbParam::setOpName(const QString& value)
	{
		m_opName = value;
	}

	bool AfbParam::visible() const
	{
		return m_visible;
	}

	void AfbParam::setVisible(bool visible)
	{
		m_visible = visible;
	}

	// Type
	//
	AfbParamType AfbParam::type() const
	{
		return m_type;
	}
	void AfbParam::setType(AfbParamType type)
	{
		m_type = type;
	}

	// Value
	//
	const QVariant& AfbParam::value() const
	{
		return m_value;
	}
	void AfbParam::setValue(const QVariant& value)
	{
		m_value = value;
	}

	// Defaut Value
	//
	const QVariant& AfbParam::defaultValue() const
	{
		return m_defaultValue;
	}
	void AfbParam::setDefaultValue(const QVariant& defaultValue)
	{
		m_defaultValue = defaultValue;
	}

	// LowLimit
	const QVariant& AfbParam::lowLimit() const
	{
		return m_lowLimit;
	}
	void AfbParam::setLowLimit(const QVariant& lowLimit)
	{
		m_lowLimit = lowLimit;
	}

	// highLimit
	//
	const QVariant& AfbParam::highLimit() const
	{
		return m_highLimit;
	}
	void AfbParam::setHighLimit(const QVariant& highLimit)
	{
		m_highLimit = highLimit;
	}

	int AfbParam::operandIndex() const
    {
		return m_operandIndex;
    }

	void AfbParam::setOperandIndex(int value)
    {
		m_operandIndex = value;
    }

	int AfbParam::size() const
    {
        return m_size;
    }

	void AfbParam::setSize(int value)
    {
        m_size = value;
    }

	bool AfbParam::instantiator() const
	{
		return m_instantiator;
	}

	void AfbParam::setInstantiator(bool value)
	{
		m_instantiator = value;
	}

	bool AfbParam::user() const
	{
		return m_user;
	}

	void AfbParam::setUser(bool value)
	{
		m_user = value;
	}

	QString AfbParam::changedScript() const
	{
		return m_changedScript;
	}

	void AfbParam::setChangedScript(const QString &value)
	{
		m_changedScript = value;
	}

    //
	//
	//							FblElement		
	//
	//

	AfbElement::AfbElement(void) :
		m_hasRam(false),
		m_requiredStart(true)
	{
	}

	AfbElement::~AfbElement(void)
	{
	}

	AfbElement::AfbElement(const AfbElement& that)
		: QObject()

	{
		*this = that;
	}

	AfbElement& AfbElement::operator=(const AfbElement& that)
	{
		if (this == &that)
		{
			return *this;
		}

		m_strID = that.m_strID;
		m_caption = that.m_caption;
		m_description = that.m_description;
		m_type = that.m_type;
		m_hasRam = that.m_hasRam;
		m_requiredStart = that.m_requiredStart;

		m_libraryScript = that.m_libraryScript;
		m_afterCreationScript = that.m_afterCreationScript;

		m_inputSignals = that.m_inputSignals;
		m_outputSignals = that.m_outputSignals;
		m_params = that.m_params;

		return *this;
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

		if (QString::compare(xmlReader->name().toString(), "ApplicationFunctionalBlocks", Qt::CaseInsensitive) != 0)
		{
			xmlReader->raiseError(QObject::tr("The file is not an ApplicationFunctionalBlocks file."));
			return !xmlReader->hasError();
		}

		if (xmlReader->readNextStartElement() == false)
		{
			return !xmlReader->hasError();
		}


		if (QString::compare(xmlReader->name().toString(), "AfbElement", Qt::CaseInsensitive) != 0)
		{
			xmlReader->raiseError(QObject::tr("AfbElement expected."));
			return !xmlReader->hasError();
		}

		if (xmlReader->attributes().hasAttribute("StrId"))
		{
			setStrID(xmlReader->attributes().value("StrId").toString());
		}

		std::vector<AfbSignal> inputSignals;
		std::vector<AfbSignal> outputSignals;
		std::vector<AfbParam> params;

		while (xmlReader->readNextStartElement())
		{
			if (QString::compare(xmlReader->name().toString(), "Properties", Qt::CaseInsensitive) == 0)
			{
				while (xmlReader->readNextStartElement())
				{
					if (QString::compare(xmlReader->name().toString(), "Caption", Qt::CaseInsensitive) == 0)
					{
						setCaption(xmlReader->readElementText());
					}

					if (QString::compare(xmlReader->name().toString(), "Description", Qt::CaseInsensitive) == 0)
					{
						setDescription(xmlReader->readElementText());
					}

					if (QString::compare(xmlReader->name().toString(), "OpCode", Qt::CaseInsensitive) == 0)
					{
						int opCode = xmlReader->readElementText().toInt();

						Afb::AfbType type;
						type.fromOpCode(opCode);

						setType(type);
					}

					if (QString::compare(xmlReader->name().toString(), "HasRam", Qt::CaseInsensitive) == 0)
					{
						setHasRam(xmlReader->readElementText() == "true" ? true : false);
					}

					if (QString::compare(xmlReader->name().toString(), "RequiredStart", Qt::CaseInsensitive) == 0)
					{
						setRequiredStart(xmlReader->readElementText() == "true" ? true : false);
					}
				}

				continue;
			}

			// Input or output signals
			//
			if (QString::compare(xmlReader->name().toString(), "OutputSignals", Qt::CaseInsensitive) == 0
					|| 	QString::compare(xmlReader->name().toString(), "InputSignals", Qt::CaseInsensitive) == 0)
			{
				enum LoadSignalType {InputSignal, OutputSignal} loadSignalType;

				if (QString::compare(xmlReader->name().toString(), "OutputSignals", Qt::CaseInsensitive) == 0)
				{
					loadSignalType = LoadSignalType::OutputSignal;
				}
				else
				{
					loadSignalType = LoadSignalType::InputSignal;
				}

				// Read signals
				//
				while (xmlReader->readNextStartElement())
				{
					if (QString::compare(xmlReader->name().toString(), "AfbElementSignal", Qt::CaseInsensitive) == 0)
					{
						AfbSignal afbSignal;
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

			if (QString::compare(xmlReader->name().toString(), "Params", Qt::CaseInsensitive) == 0)
			{
				// Read params
				//
				while (xmlReader->readNextStartElement())
				{
					if (QString::compare(xmlReader->name().toString(), "AfbElementParam", Qt::CaseInsensitive) == 0)
					{
						AfbParam afbParam;
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

			if (QString::compare(xmlReader->name().toString(), "CommonScript", Qt::CaseInsensitive) == 0)
			{
				// Read params
				//
				while (xmlReader->readNextStartElement())
				{
					if (QString::compare(xmlReader->name().toString(), "Library", Qt::CaseInsensitive) == 0)
					{
						setLibraryScript(xmlReader->readElementText());
					}
					else
					{
						if (QString::compare(xmlReader->name().toString(), "AfterCreation", Qt::CaseInsensitive) == 0)
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
		xmlWriter->writeTextElement("Description", description());
		xmlWriter->writeTextElement("OpCode", QString::number(type().toOpCode()));
		xmlWriter->writeTextElement("HasRam", hasRam() ? "true" : "false");
		xmlWriter->writeTextElement("RequiredStart", requiredStart() ? "true" : "false");
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

	QObject* AfbElement::getAfbSignalByOpIndex(int opIndex)
	{
		for (AfbSignal& s : m_inputSignals)
		{
			if (s.operandIndex() == opIndex)
			{
				return &s;
			}
		}
		for (AfbSignal& s : m_outputSignals)
		{
			if (s.operandIndex() == opIndex)
			{
				return &s;
			}
		}
		return nullptr;
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

	void AfbElement::updateParams(const std::vector<AfbParam>& params)
	{
		std::vector<AfbParam> newParams;
		newParams.reserve(params.size());

		for (const AfbParam& p : params)
		{
			std::vector<AfbParam>::iterator found = std::find_if(m_params.begin(), m_params.end(),
				[&p](const AfbParam& mp)
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

	QString AfbElement::description() const
	{
		return m_description;
	}

	void AfbElement::setDescription(const QString& value)
	{
		m_description = value;
	}

	// Type - Opcode
	//

	const Afb::AfbType& AfbElement::type() const
	{
		return m_type;
	}

	Afb::AfbType& AfbElement::type()
	{
		return m_type;
	}

	void AfbElement::setType(const Afb::AfbType& value)
	{
		m_type = value;
	}

	bool AfbElement::hasRam() const
	{
		return m_hasRam;
	}

	void AfbElement::setHasRam(bool value)
	{
		m_hasRam = value;
	}

	bool AfbElement::requiredStart() const
	{
		return m_requiredStart;
	}

	void AfbElement::setRequiredStart(bool value)
	{
		m_requiredStart = value;
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
	const std::vector<AfbSignal>& AfbElement::inputSignals() const
	{
		return m_inputSignals;
	}
	void AfbElement::setInputSignals(const std::vector<AfbSignal>& inputsignals)
	{
		m_inputSignals = inputsignals;
	}

	// OutputSignals
	//
	const std::vector<AfbSignal>& AfbElement::outputSignals() const
	{
		return m_outputSignals;
	}
	void AfbElement::setOutputSignals(const std::vector<AfbSignal>& outputsignals)
	{
		m_outputSignals = outputsignals;
	}

	// Params
	//
	const std::vector<AfbParam>& AfbElement::params() const
	{
		return m_params;
	}

	std::vector<AfbParam>& AfbElement::params()
	{
		return m_params;
	}

	int AfbElement::paramsCount() const
	{
		return static_cast<int>(m_params.size());
	}

	void AfbElement::setParams(const std::vector<AfbParam>& params)
	{
		m_params = params;
	}


	QString AfbElement::instantiatorID() const
	{
		if (!m_instantiatorID.isEmpty())
		{
			return m_instantiatorID;

		}

		m_instantiatorID = m_strID;

		QVector<int> instantiatorParamIndex;

		for(AfbParam param : params())
		{
			if (param.instantiator())
			{
				instantiatorParamIndex.append(param.operandIndex());
			}
		}

		// sort instantiator param's indexes by ascending
		//

		int count = instantiatorParamIndex.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int j = i + 1; j < count; j++)
			{
				if (instantiatorParamIndex[i] > instantiatorParamIndex[j])
				{
					int tmp = instantiatorParamIndex[i];
					instantiatorParamIndex[i] = instantiatorParamIndex[j];
					instantiatorParamIndex[j] = tmp;
				}
			}
		}

		// append instantiator param's values to instantiatorID
		//

		for(int paramIndex : instantiatorParamIndex)
		{
			for(AfbParam param : params())
			{
				if (paramIndex == param.operandIndex())
				{
					m_instantiatorID += QString(":%1").arg(param.value().toString());
					break;
				}
			}
		}

		return m_instantiatorID;
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
			std::shared_ptr<Afb::AfbElement> e = std::make_shared<Afb::AfbElement>();
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

