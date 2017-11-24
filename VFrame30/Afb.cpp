#include "Afb.h"
#include <QDomElement>
#include <QXmlStreamReader>

namespace Afb
{

	AfbComponentPin::AfbComponentPin(const QString caption, int opIndex, AfbComponentPinType type) :
		m_caption(caption),
		m_opIndex(opIndex),
		m_type(type)
	{
	}

	bool AfbComponentPin::loadFromXml(const QDomElement& xmlElement, QString* errorMessage)
	{
		if (errorMessage == nullptr ||
			xmlElement.isNull() == true ||
			xmlElement.tagName() != QLatin1String("Pin"))
		{
			assert(errorMessage);
			assert(xmlElement.isNull() == false);
			assert(xmlElement.tagName() == QLatin1String("Pin"));
			return false;
		}

		// xmlElement suppose to contain xml like:
		// <Pin OpName="i_oprd_quant"	OpIndex ="0"	PinType = "Param">
		//

		// OpName -> m_caption
		//
		if (xmlElement.hasAttribute(QLatin1String("OpName")) == false)
		{
			*errorMessage = "AFBCompoment\\Pin does not have attribute OpName";
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("OpName"));

		// OpIndex -> m_opIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("OpIndex")) == false)
		{
			*errorMessage = "AFBCompoment\\Pin does not have attribute OpIndex";
			return false;
		}

		m_opIndex = xmlElement.attribute(QLatin1String("OpIndex")).toInt();

		// PinType -> m_type
		//
		if (xmlElement.hasAttribute(QLatin1String("PinType")) == false)
		{
			*errorMessage = "AFBCompoment\\Pin does not have attribute PinType";
			return false;
		}

		QString typeStr = xmlElement.attribute(QLatin1String("PinType"));

		if (typeStr.compare(QLatin1String("Param"), Qt::CaseInsensitive) == 0)
		{
			m_type = AfbComponentPinType::Param;
		}

		if (typeStr.compare(QLatin1String("Input"), Qt::CaseInsensitive) == 0)
		{
			m_type = AfbComponentPinType::Input;
		}

		if (typeStr.compare(QLatin1String("Output"), Qt::CaseInsensitive) == 0)
		{
			m_type = AfbComponentPinType::Output;
		}

		// --
		//

		return true;
	}

	bool AfbComponentPin::saveToXml(QDomElement* /*xmlElement*/) const
	{
		// To do
		//
		assert(false);
		return false;
	}

	QString AfbComponentPin::caption() const
	{
		return m_caption;
	}

	void AfbComponentPin::setCaption(const QString& value)
	{
		m_caption = value;
	}

	int AfbComponentPin::opIndex() const
	{
		return m_opIndex;
	}

	void AfbComponentPin::setOpIndex(int value)
	{
		m_opIndex = value;
	}

	AfbComponentPinType AfbComponentPin::type() const
	{
		return m_type;
	}

	void AfbComponentPin::setType(AfbComponentPinType value)
	{
		m_type = value;
	}

	bool AfbComponentPin::isInputOrParam() const
	{
		return	m_type == AfbComponentPinType::Input ||
				m_type == AfbComponentPinType::Param;
	}

	bool AfbComponentPin::isOutput() const
	{
		return	m_type == AfbComponentPinType::Output;
	}

	//
	//				AfbComponent
	//
	AfbComponent::AfbComponent()
	{
	}

	bool AfbComponent::loadFromXml(const QDomElement& xmlElement, QString* errorMessage)
	{
		if (errorMessage == nullptr ||
			xmlElement.isNull() == true ||
			xmlElement.tagName() != QLatin1String("AFBComponent"))
		{
			assert(errorMessage);
			assert(xmlElement.isNull() == false);
			assert(xmlElement.tagName() == QLatin1String("AFBComponent"));
			return false;
		}

		// Caption
		//
		if (xmlElement.hasAttribute(QLatin1String("Caption")) == false)
		{
			*errorMessage = "AFBCompoment does not have attribute Caption";
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("Caption"));

		// OpCode
		//
		if (xmlElement.hasAttribute(QLatin1String("OpCode")) == false)
		{
			*errorMessage = QString("AFBCompoment %1 does not have attribute OpCode").arg(m_caption);
			return false;
		}

		m_opCode = xmlElement.attribute(QLatin1String("OpCode")).toInt();

		// ImpVersion
		//
		if (xmlElement.hasAttribute(QLatin1String("ImpVersion")) == false)
		{
			*errorMessage = QString("AFBCompoment %1 does not have attribute ImpVersion").arg(m_caption);
			return false;
		}

		m_impVersion = xmlElement.attribute(QLatin1String("ImpVersion")).toInt();

		// VersionOpIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("VersionOpIndex")) == false)
		{
			*errorMessage = QString("AFBCompoment %1 does not have attribute VersionOpIndex").arg(m_caption);
			return false;
		}

		m_versionOpIndex = xmlElement.attribute(QLatin1String("VersionOpIndex")).toInt();

		// MaxInstCount
		//
		if (xmlElement.hasAttribute(QLatin1String("MaxInstCount")) == false)
		{
			*errorMessage = QString("AFBCompoment %1 does not have attribute MaxInstCount").arg(m_caption);
			return false;
		}

		m_maxInstCount = xmlElement.attribute(QLatin1String("MaxInstCount")).toInt();

		// Pins
		//
		{
			QDomElement p = xmlElement.firstChildElement(QLatin1String("Pin"));
			m_pins.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbComponentPin pin;

				bool ok = pin.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(QString(", Component %1").arg(m_caption));
					return false;
				}

				m_pins[pin.opIndex()] = pin;

				p = p.nextSiblingElement(QLatin1String("Pin"));
			}
		}


		return true;
	}

	bool AfbComponent::saveToXml(QDomElement* /*xmlElement*/) const
	{
		// To Do
		//
		assert(false);
		return false;
	}

	int AfbComponent::opCode() const
	{
		return m_opCode;
	}

	void AfbComponent::setOpCode(int value)
	{
		m_opCode = value;
	}

	QString AfbComponent::caption() const
	{
		return m_caption;
	}

	void AfbComponent::setCaption(const QString& value)
	{
		m_caption = value;
	}

	int AfbComponent::impVersion() const
	{
		return m_impVersion;
	}

	void AfbComponent::setImpVersion(int value)
	{
		m_impVersion = value;
	}

	int AfbComponent::versionOpIndex() const
	{
		return m_versionOpIndex;
	}

	void AfbComponent::setVersionOpIndex(int value)
	{
		m_versionOpIndex = value;
	}

	int AfbComponent::maxInstCount() const
	{
		return m_maxInstCount;
	}

	void AfbComponent::setMaxInstCount(int value)
	{
		m_maxInstCount = value;
	}


	//
	//							AfbSignal
	//
	AfbSignal::AfbSignal(void)
	{
	}

	AfbSignal::AfbSignal(const AfbSignal& that)
	{
		*this = that;
	}

	AfbSignal& AfbSignal::operator=(const AfbSignal& that) noexcept
	{
		if (this == &that)
		{
			return *this;
		}

		m_opName = that.m_opName;
		m_caption = that.m_caption;
		m_type = that.m_type;
		m_dataFormat = that.m_dataFormat;
		m_operandIndex = that.m_operandIndex;
		m_size = that.m_size;
		m_byteOrder = that.m_byteOrder;
		m_busDataFormat = that.m_busDataFormat;

		return *this;
	}


	bool AfbSignal::loadFromXml(const QDomElement& xmlElement, QString* errorMessage)
	{
		if (errorMessage == nullptr ||
				xmlElement.isNull() == true ||
				xmlElement.tagName() != QLatin1String(QLatin1String("Pin")))
		{
			assert(errorMessage);
			assert(xmlElement.isNull() == false);
			assert(xmlElement.tagName() == QLatin1String("Pin"));
			return false;
		}

		// OpName
		//
		if (xmlElement.hasAttribute(QLatin1String("OpName")) == false)
		{
			*errorMessage = QString("Can't find attribute OpName.");
			return false;
		}

		m_opName = xmlElement.attribute(QLatin1String("OpName"));

		// Caption
		//
		if (xmlElement.hasAttribute(QLatin1String("Caption")) == false)
		{
			*errorMessage = QString("Can't find attribute Caption.");
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("Caption"));

		// Type
		//
		if (xmlElement.hasAttribute(QLatin1String("Type")) == false)
		{
			*errorMessage = QString("Can't find attribute Type. Pin %1").arg(m_caption);
			return false;
		}
		else
		{
			QString typeAttribute = xmlElement.attribute(QLatin1String("Type"));

			bool ok = setType(typeAttribute);
			if (ok == false)
			{
				*errorMessage = QString("Unknown SignalType %1. Pin %2").arg(typeAttribute).arg(m_caption);
				return false;
			}
		}

		// DataFormat
		//
		if (xmlElement.hasAttribute(QLatin1String("DataFormat")) == false)
		{
			if (type() == E::SignalType::Analog)		// Ignore for discretes
			{
				*errorMessage = QString("Can't find attribute DataFormat. Pin %1").arg(m_caption);
				return false;
			}
		}
		else
		{
			QString dataFormatAttribute = xmlElement.attribute(QLatin1String("DataFormat"));

			bool ok = setDataFormat(dataFormatAttribute);
			if (ok == false)
			{
				*errorMessage = QString("Unknown DataFormat %1. Pin %2").arg(dataFormatAttribute).arg(m_caption);
				return false;
			}
		}

		// OpIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("OpIndex")) == false)
		{
			*errorMessage = QString("Can't find attribute OpIndex. Pin %1").arg(m_caption);
			return false;
		}

		m_operandIndex = xmlElement.attribute(QLatin1String("OpIndex")).toInt();

		// Size
		//
		if (xmlElement.hasAttribute(QLatin1String("Size")) == false)
		{
			*errorMessage = QString("Can't find attribute Size. Pin %1").arg(m_caption);
			return false;
		}

		m_size= xmlElement.attribute(QLatin1String("Size")).toInt();

		// ByteOrder
		//
		if (xmlElement.hasAttribute(QLatin1String("ByteOrder")) == true)
		{
			QString byteOrderAttribute = xmlElement.attribute(QLatin1String("ByteOrder"));

			bool ok = setByteOrder(byteOrderAttribute);
			if (ok == false)
			{
				*errorMessage = QString("Unknown ByteOrder %1. Pin %2").arg(byteOrderAttribute).arg(m_caption);
				return false;
			}
		}

		// BusDataFormat
		//
		if (xmlElement.hasAttribute(QLatin1String("BusDataFormat")) == false)
		{
			if (type() == E::SignalType::Bus)
			{
				*errorMessage = QString("Not set BusDataFormat for pin %2").arg(m_caption);
				return false;
			}
		}
		else
		{
			QString attribute = xmlElement.attribute(QLatin1String("BusDataFormat"));

			bool ok = setBusDataFormat(attribute);
			if (ok == false)
			{
				*errorMessage = QString("Unknown BusDataFormat %1. Pin %2").arg(attribute).arg(m_caption);
				return false;
			}
		}

		return true;
	}

	bool AfbSignal::saveToXml(QDomElement* element) const
	{
		if (element == nullptr)
		{
			assert(element);
			return false;
		}

		element->setTagName(QLatin1String("Pin"));

		// OpName
		//
		element->setAttribute(QLatin1String("OpName"), m_opName);

		// Caption
		//
		element->setAttribute(QLatin1String("Caption"), m_caption);

		// Type
		//
		element->setAttribute(QLatin1String("Type"), E::valueToString(m_type));

		// DataFormat
		//
		element->setAttribute(QLatin1String("DataFormat"), E::valueToString(m_dataFormat));

		// OpIndex
		//
		element->setAttribute(QLatin1String("OpIndex"), m_operandIndex);

		// Size
		//
		element->setAttribute(QLatin1String("Size"), m_size);

		// ByteOrder
		//
		element->setAttribute(QLatin1String("ByteOrder"), E::valueToString(m_byteOrder));

		// BusDataFormat
		//
		element->setAttribute(QLatin1String("BusDataFormat"), E::valueToString(m_busDataFormat));

		return true;
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
	QString AfbSignal::caption() const
	{
		return m_caption;
	}
	void AfbSignal::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Type
	//
	E::SignalType AfbSignal::type() const
	{
		return m_type;
	}
	int AfbSignal::jsType() const
	{
		return static_cast<int>(m_type);
	}
	void AfbSignal::setType(E::SignalType type)
	{
		m_type = type;
	}

	bool AfbSignal::setType(const QString& type)
	{
		bool ok = false;
		m_type = E::stringToValue<E::SignalType>(type, &ok);
		return ok;
	}

	E::DataFormat AfbSignal::dataFormat() const
	{
		return m_dataFormat;
	}

	void AfbSignal::setDataFormat(E::DataFormat dataFormat)
	{
		m_dataFormat = dataFormat;
	}

	bool AfbSignal::setDataFormat(const QString& dataFormat)
	{
		bool ok = false;
		m_dataFormat = E::stringToValue<E::DataFormat>(dataFormat, &ok);
		return ok;
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

	E::ByteOrder AfbSignal::byteOrder() const
	{
		return m_byteOrder;
	}

	void AfbSignal::setByteOrder(E::ByteOrder value)
	{
		m_byteOrder = value;
	}

	bool AfbSignal::setByteOrder(const QString& value)
	{
		bool ok = false;
		m_byteOrder = E::stringToValue<E::ByteOrder>(value, &ok);
		return ok;
	}

	bool AfbSignal::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool AfbSignal::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
	}

	bool AfbSignal::isBus() const
	{
		return m_type == E::SignalType::Bus;
	}

	E::BusDataFormat AfbSignal::busDataFormat() const
	{
		return m_busDataFormat;
	}

	void AfbSignal::setBusDataFormat(E::BusDataFormat value)
	{
		m_busDataFormat = value;
	}

	bool AfbSignal::setBusDataFormat(const QString& value)
	{
		bool ok = false;
		m_busDataFormat = E::stringToValue<E::BusDataFormat>(value, &ok);
		return ok;
	}

	//
	//
	//							AfbParam
	//
	//

	AfbParam::AfbParam(void):
		m_visible(true),
		m_type(E::SignalType::Analog),
		m_dataFormat(E::DataFormat::UnsignedInt),
		m_byteOrder(E::ByteOrder::BigEndian),
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

	void AfbParam::update(const E::SignalType& type, const E::DataFormat dataFormat, E::ByteOrder byteOrder, const QVariant &lowLimit, const QVariant &highLimit)
	{
		m_type = type;
		m_dataFormat = dataFormat;
		m_byteOrder = byteOrder,
		m_lowLimit = lowLimit;
		m_highLimit = highLimit;

		return;
	}

	bool AfbParam::deprecatedLoadFromXml(QXmlStreamReader* xmlReader)
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
			if (QString::compare(xmlReader->attributes().value("Type").toString(), "Analog", Qt::CaseInsensitive) == 0)
			{
				setType(E::SignalType::Analog);
			}
			else
				if (QString::compare(xmlReader->attributes().value("Type").toString(), "Discrete", Qt::CaseInsensitive) == 0)
				{
					setType(E::SignalType::Discrete);
				}
				else
				{
					xmlReader->raiseError(QObject::tr("AfbElementParam, unknown Type"));
					return false;
				}
		}

		if (xmlReader->attributes().hasAttribute("DataFormat"))
		{
			if (QString::compare(xmlReader->attributes().value("DataFormat").toString(), "UnsignedInt", Qt::CaseInsensitive) == 0)
			{
				setDataFormat(E::DataFormat::UnsignedInt);
			}
			else
				if (QString::compare(xmlReader->attributes().value("DataFormat").toString(), "SignedInt", Qt::CaseInsensitive) == 0)
				{
					setDataFormat(E::DataFormat::SignedInt);
				}
				else
					if (QString::compare(xmlReader->attributes().value("DataFormat").toString(), "Float", Qt::CaseInsensitive) == 0)
					{
						setDataFormat(E::DataFormat::Float);
					}
					else
					{
						xmlReader->raiseError(QObject::tr("AfbElementParam, unknown DataFormat"));
						return false;
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


			if (QString::compare(valueName, "Value", Qt::CaseInsensitive) == 0 ||
					QString::compare(valueName, "Default", Qt::CaseInsensitive) == 0 ||
					QString::compare(valueName, "LowLimit", Qt::CaseInsensitive) == 0 ||
					QString::compare(valueName, "HighLimit", Qt::CaseInsensitive) == 0)
			{
				QString str = xmlReader->readElementText();
				QVariant val;

				if (isAnalog())
				{
					switch (dataFormat())
					{
					case E::DataFormat::UnsignedInt:
					case E::DataFormat::SignedInt:
					{
						val = str.toInt();
						break;
					}
					case E::DataFormat::Float:
					{
						val = str.toDouble();
						break;
					}
					default:
						assert(false);
					}
				}
				else
				{
					val = str == "1" ? true : false;
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

	bool AfbParam::loadFromXml(const QDomElement& xmlElement, QString* errorMessage)
	{
		if (errorMessage == nullptr ||
				xmlElement.isNull() == true ||
				xmlElement.tagName() != QLatin1String("Param"))
		{
			assert(errorMessage);
			assert(xmlElement.isNull() == false);
			assert(xmlElement.tagName() == QLatin1String("Param"));
			return false;
		}

		// OpName
		//
		if (xmlElement.hasAttribute(QLatin1String("OpName")) == false)
		{
			*errorMessage = "Can't find attribute OpName.";
			return false;
		}

		m_opName = xmlElement.attribute(QLatin1String("OpName"));

		// Caption
		//
		if (xmlElement.hasAttribute(QLatin1String("Caption")) == false)
		{
			*errorMessage = "Can't find attribute Caption.";
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("Caption"));

		// Visible
		//
		if (xmlElement.hasAttribute(QLatin1String("Visible")) == false)
		{
			*errorMessage = "Can't find attribute Visible.";
			return false;
		}

		m_visible = xmlElement.attribute(QLatin1String("Visible")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

		// OpIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("OpIndex")) == false)
		{
			*errorMessage = "Can't find attribute OpIndex.";
			return false;
		}

		m_operandIndex = xmlElement.attribute(QLatin1String("OpIndex")).toInt();

		// Size
		//
		if (xmlElement.hasAttribute(QLatin1String("Size")) == false)
		{
			*errorMessage = "Can't find attribute Size.";
			return false;
		}

		m_size = xmlElement.attribute(QLatin1String("Size")).toInt();

		// Instantiator
		//
		if (xmlElement.hasAttribute(QLatin1String("Instantiator")) == false)
		{
			*errorMessage = "Can't find attribute Instantiator.";
			return false;
		}

		m_instantiator = xmlElement.attribute(QLatin1String("Instantiator")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

		// User
		//
		if (xmlElement.hasAttribute(QLatin1String("User")) == false)
		{
			*errorMessage = "Can't find attribute User.";
			return false;
		}

		m_user = xmlElement.attribute(QLatin1String("User")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;

		// Type
		//
		if (xmlElement.hasAttribute(QLatin1String("Type")) == false)
		{
			*errorMessage = QString("Can't find attribute Type. Pin %1").arg(m_caption);
			return false;
		}

		QString typeAttribute = xmlElement.attribute(QLatin1String("Type"));

		if (typeAttribute.compare(QLatin1String(QLatin1String("Analog")), Qt::CaseInsensitive) == 0)
		{
			m_type = E::SignalType::Analog;
		}
		else
		{
			if (typeAttribute.compare(QLatin1String(QLatin1String("Discrete")), Qt::CaseInsensitive) == 0)
			{
				m_type = E::SignalType::Discrete;
			}
			else
			{
				*errorMessage = QString("Unknown SignalType %1. Param %2").arg(typeAttribute).arg(m_caption);
				return false;
			}
		}

		// DataFormat
		//
		if (isAnalog() == true)
		{
			if (xmlElement.hasAttribute(QLatin1String("DataFormat")) == false)
			{
				*errorMessage = QString("Can't find attribute DataFormat. Param %1").arg(m_caption);
				return false;
			}

			QString dataFormatAttribute = xmlElement.attribute(QLatin1String("DataFormat"));

			if (dataFormatAttribute.compare(QLatin1String("UnsignedInt"), Qt::CaseInsensitive) == 0)
			{
				m_dataFormat = E::DataFormat::UnsignedInt;
			}
			else
			{
				if (dataFormatAttribute.compare(QLatin1String("SignedInt"), Qt::CaseInsensitive) == 0)
				{
					m_dataFormat = E::DataFormat::SignedInt;
				}
				else
				{
					if (dataFormatAttribute.compare(QLatin1String("Float"), Qt::CaseInsensitive) == 0)
					{
						m_dataFormat = E::DataFormat::Float;
					}
					else
					{
						*errorMessage = QString("Unknown DataFormat %1. Param %2").arg(dataFormatAttribute).arg(m_caption);
						return false;
					}
				}
			}
		}

		// ByteOrder
		//
		if (xmlElement.hasAttribute(QLatin1String("ByteOrder")) == true)
		{
			QString byteOrderAttribute = xmlElement.attribute(QLatin1String("ByteOrder"));

			if (byteOrderAttribute.compare(QLatin1String("LittleEndian"), Qt::CaseInsensitive) == 0)
			{
				m_byteOrder = E::ByteOrder::LittleEndian;
			}
			else
			{
				if (byteOrderAttribute.compare(QLatin1String("BigEndian"), Qt::CaseInsensitive) == 0)
				{
					m_byteOrder = E::ByteOrder::BigEndian;
				}
				else
				{
					*errorMessage = QString("Unknown ByteOrder %1. Pin %2").arg(byteOrderAttribute).arg(m_caption);
					return false;
				}
			}
		}


		// Section <Value>
		//
		std::function<QVariant(QString, E::SignalType, E::DataFormat)> valToDataFormat =
				[](QString str, E::SignalType type, E::DataFormat dataFormat) -> QVariant
		{
			if (type == E::SignalType::Analog)
			{
				switch (dataFormat)
				{
				case E::DataFormat::UnsignedInt:
				case E::DataFormat::SignedInt:
					return QVariant(str.toInt());
				case E::DataFormat::Float:
					return QVariant(str.toDouble());
				default:
					assert(false);
					return QVariant();
				}
			}

			if (type == E::SignalType::Discrete)
			{
				return QVariant(str == "1" ? true : false);
			}

			assert(false);
			return QVariant();
		};

		{
			QDomElement valueElement = xmlElement.firstChildElement(QLatin1String("Value"));

			if (valueElement.isNull() == true)
			{
				*errorMessage = QString("Can't find section Value. Param %1.").arg(m_caption);
				return false;
			}

			QVariant v = valToDataFormat(valueElement.text(), type(), dataFormat());
			setValue(v);
		}

		// Section <Default>
		//
		{
			QDomElement defaultElement = xmlElement.firstChildElement(QLatin1String("Default"));

			if (user() == true &&
					defaultElement.isNull() == true)
			{
				*errorMessage = QString("Can't find section Default. Param %1.").arg(m_caption);
				return false;
			}

			if (defaultElement.isNull() == false)
			{
				QVariant v = valToDataFormat(defaultElement.text(), type(), dataFormat());
				setDefaultValue(v);
			}
		}

		if (isAnalog() == true)
		{
			// Section <LowLimit>
			//
			{
				QDomElement e = xmlElement.firstChildElement(QLatin1String("LowLimit"));

				if (user() == true &&
						e.isNull() == true)
				{
					*errorMessage = QString("Can't find section LowLimit. Param %1.").arg(m_caption);
					return false;
				}

				if (e.isNull() == false)
				{
					QVariant v = valToDataFormat(e.text(), type(), dataFormat());
					setLowLimit(v);
				}
			}

			// Section <HighLimit>
			//
			{
				QDomElement e = xmlElement.firstChildElement(QLatin1String("HighLimit"));

				if (user() == true &&
						e.isNull() == true)
				{
					*errorMessage = QString("Can't find section HighLimit. Param %1.").arg(m_caption);
					return false;
				}

				if (e.isNull() == false)
				{
					QVariant v = valToDataFormat(e.text(), type(), dataFormat());
					setHighLimit(v);
				}
			}
		}

		// Section <Units>
		//
		{
			QDomElement e = xmlElement.firstChildElement(QLatin1String("Units"));

			if (e.isNull() == true)
			{
				m_units.clear();
			}
			else
			{
				m_units = e.text();
			}
		}


		// Section <Script>::<Changed>
		//
		m_changedScript.clear();

		{
			QDomElement s = xmlElement.firstChildElement(QLatin1String("Script"));

			if (s.isNull() == false)
			{
				QDomElement sc = s.firstChildElement(QLatin1String("Changed"));

				if (sc.isNull() == false)
				{
					m_changedScript = sc.text().trimmed();
				}
			}
		}

		return true;
	}

	bool AfbParam::saveToXml(QDomElement* xmlElement) const
	{
		if (xmlElement == nullptr ||
				xmlElement->isNull() == true)
		{
			assert(xmlElement);
			assert(xmlElement->isNull() == false);
			return false;
		}

		xmlElement->setTagName(QLatin1String("Param"));

		// OpName
		//
		xmlElement->setAttribute(QLatin1String("OpName"), m_opName);

		// Caption
		//
		xmlElement->setAttribute(QLatin1String("Caption"), m_caption);

		// Visible
		//
		xmlElement->setAttribute(QLatin1String("Visible"), m_visible ? QLatin1String("true") : QLatin1String("false"));

		// OpIndex
		//
		xmlElement->setAttribute(QLatin1String("OpIndex"), m_operandIndex);

		// Size
		//
		xmlElement->setAttribute(QLatin1String("Size"), m_size);

		// Instantiator
		//
		xmlElement->setAttribute(QLatin1String("Instantiator"), m_instantiator ? QLatin1String("true") : QLatin1String("false"));

		// User
		//
		xmlElement->setAttribute(QLatin1String("User"), m_user ? QLatin1String("true") : QLatin1String("false"));

		// Type
		//
		xmlElement->setAttribute(QLatin1String("Type"), E::valueToString(m_type));

		// DataFormat
		//
		xmlElement->setAttribute(QLatin1String("DataFormat"), E::valueToString(m_dataFormat));

		// ByteOrder
		//
		xmlElement->setAttribute(QLatin1String("ByteOrder"), E::valueToString(m_byteOrder));

		// Sections "Values"
		//
		QDomDocument doc = xmlElement->ownerDocument();
		assert(doc.isNull() == false);

		if (isDiscrete() == true)
		{
			// Value
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Value")));
				QDomText text = doc.createTextNode(value() == true ? "1" : "0");
				node.appendChild(text);
			}
			// Default
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Default")));
				QDomText text = doc.createTextNode(defaultValue() == true ? "1" : "0");
				node.appendChild(text);
			}
			// LowLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("LowLimit")));
				QDomText text = doc.createTextNode(lowLimit() == true ? "1" : "0");
				node.appendChild(text);
			}
			// HighLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("HighLimit")));
				QDomText text = doc.createTextNode(highLimit() == true ? "1" : "0");
				node.appendChild(text);
			}
		}
		else
		{
			// Value
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Value")));
				QDomText text = doc.createTextNode(value().toString());
				node.appendChild(text);
			}
			// Default
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Default")));
				QDomText text = doc.createTextNode(defaultValue().toString());
				node.appendChild(text);
			}
			// LowLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("LowLimit")));
				QDomText text = doc.createTextNode(lowLimit().toString());
				node.appendChild(text);
			}
			// HighLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("HighLimit")));
				QDomText text = doc.createTextNode(highLimit().toString());
				node.appendChild(text);
			}
		}

		// Section <Units>
		//
		{
			QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Units")));
			QDomText text = doc.createTextNode(m_units);
			node.appendChild(text);
		}

		// Section <Script>
		//
		{
			QDomNode scriptNode = xmlElement->appendChild(doc.createElement(QLatin1String("Script")));

			// Section <Changed>
			//
			{
				QDomNode scriptChaneNode = scriptNode.appendChild(doc.createElement(QLatin1String("Changed")));
				QDomText text = doc.createTextNode(m_changedScript);
				scriptChaneNode.appendChild(text);
			}
		}

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
	E::SignalType AfbParam::type() const
	{
		return m_type;
	}
	void AfbParam::setType(E::SignalType type)
	{
		m_type = type;
	}

	E::DataFormat AfbParam::dataFormat() const
	{
		return m_dataFormat;
	}

	void AfbParam::setDataFormat(E::DataFormat dataFormat)
	{
		m_dataFormat = dataFormat;
	}

	bool AfbParam::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool AfbParam::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
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
	//
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

	E::ByteOrder AfbParam::byteOrder() const
	{
		return m_byteOrder;
	}

	void AfbParam::setByteOrder(E::ByteOrder value)
	{
		m_byteOrder = value;
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
		m_changedScript = value.trimmed();
	}

	QString AfbParam::units() const
	{
		return m_units;
	}

	void AfbParam::setUnits(const QString& value)
	{
		m_units = value;
	}

	//
	//
	//							FblElement
	//
	//
	AfbElement::AfbElement(void)
	{
	}

	AfbElement::AfbElement(const AfbElement& that)
		: QObject()

	{
		*this = that;
	}

	AfbElement& AfbElement::operator=(const AfbElement& that) noexcept
	{
		if (this == &that)
		{
			return *this;
		}

		// Copy via xml serialization is TOO SLOW,
		// and it's slows down compilation (ordering items).
		// If ordering items (struct AppLogicItem) is rewriten without copy,
		// then it will be possible to uncomment  xml serialization
		//

//		QDomDocument d;
//		QDomElement e = d.createElement(QLatin1String("AFB"));
//		that.saveToXml(&e);

//		QString errorMessage;
//		this->loadFromXml(e, &errorMessage);

//		this->m_component = that.m_component;

		// Member by member copy
		//
		m_strId = that.m_strId;
		m_caption = that.m_caption;
		m_description = that.m_description;
		m_version = that.m_version;
		m_category = that.m_category;
		m_opCode = that.m_opCode;
		m_hasRam = that.m_hasRam;
		m_internalUse = that.m_internalUse;
		m_minWidth = that.m_minWidth;
		m_minHeight = that.m_minHeight;

		m_libraryScript = that.m_libraryScript;
		m_afterCreationScript = that.m_afterCreationScript;

		m_inputSignals = that.m_inputSignals;
		m_outputSignals = that.m_outputSignals;

		m_params = that.m_params;

		m_component = that.m_component;

		return *this;
	}

	bool AfbElement::loadFromXml(const Proto::AfbElementXml& data, QString* errorMsg)
	{
		QByteArray ba(data.data().data(), static_cast<int>(data.data().size()));

		QDomDocument doc;

		bool result = doc.setContent(ba, errorMsg);
		if (result == false)
		{
			return false;
		}

		result = loadFromXml(doc.firstChild().toElement(), errorMsg);

		return result;
	}

	bool AfbElement::loadFromXml(const QDomElement& xmlElement, QString* errorMessage)
	{
		if (errorMessage == nullptr ||
				xmlElement.isNull() == true ||
				xmlElement.tagName() != QLatin1String("AFB"))
		{
			assert(errorMessage);
			assert(xmlElement.isNull() == false);
			assert(xmlElement.tagName() == QLatin1String("AFB"));
			return false;
		}

		// id
		//
		if (xmlElement.hasAttribute(QLatin1String("id")) == false)
		{
			*errorMessage = tr("Cant find attribute id.");
			return false;
		}

		m_strId = xmlElement.attribute(QLatin1String("id"));

		// Caption
		//
		if (xmlElement.hasAttribute(QLatin1String("Caption")) == false)
		{
			*errorMessage = tr("Cant find attribute Caption. AFB %1").arg(m_strId);
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("Caption"));

		// Version
		//
		if (xmlElement.hasAttribute(QLatin1String("Version")) == false)
		{
			*errorMessage = tr("Cant find attribute Version. AFB %1").arg(m_strId);
			return false;
		}

		m_version = xmlElement.attribute(QLatin1String("Version"));

		// Section Properties
		//
		{
			QDomElement properties = xmlElement.firstChildElement(QLatin1String("Properties"));

			if (properties.isNull() == true)
			{
				*errorMessage = tr("Cant find section Properties. AFB %1").arg(m_strId);
				return false;
			}

			// Section Properties::Description
			//
			{
				QDomElement description = properties.firstChildElement(QLatin1String("Description"));

				if (description.isNull() == true)
				{
					*errorMessage = tr("Cant find section Description. AFB %1").arg(m_strId);
					return false;
				}

				m_description = description.text();
			}

			// Section Properties::Category
			//
			{
				QDomElement category = properties.firstChildElement(QLatin1String("Category"));

				if (category.isNull() == true)
				{
					*errorMessage = tr("Cant find section Category. AFB %1").arg(m_strId);
					return false;
				}

				m_category = category.text();
			}

			// Section Properties::OpCode
			//
			{
				QDomElement opCode = properties.firstChildElement(QLatin1String("OpCode"));

				if (opCode.isNull() == true)
				{
					*errorMessage = tr("Cant find section OpCode. AFB %1").arg(m_strId);
					return false;
				}

				m_opCode = opCode.text().toInt();
			}
			//m_type.fromOpCode(m_opCode);

			// Section Properties::HasRam
			//
			{
				QDomElement hasRam = properties.firstChildElement(QLatin1String("HasRam"));

				if (hasRam.isNull() == true)
				{
					*errorMessage = tr("Cant find section HasRam. AFB %1").arg(m_strId);
					return false;
				}

				m_hasRam = hasRam.text().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
			}

			// Section Properties::InternalUse
			//
			{
				QDomElement internalUse = properties.firstChildElement(QLatin1String("InternalUse"));

				if (internalUse.isNull() == true)
				{
					*errorMessage = tr("Cant find section InternalUse. AFB %1").arg(m_strId);
					return false;
				}

				m_internalUse = internalUse.text().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
			}

			// Section Properties::MinWidth
			//
			{
				QDomElement minWidth = properties.firstChildElement(QLatin1String("MinWidth"));

				if (minWidth.isNull() == true)
				{
					m_minWidth = 10;
				}
				else
				{
					m_minWidth = qBound(10, minWidth.text().toInt(), 100);
				}
			}

			// Section Properties::MinHeight
			//
			{
				QDomElement minHeight = properties.firstChildElement(QLatin1String("MinHeight"));

				if (minHeight.isNull() == true)
				{
					m_minHeight = 4;
				}
				else
				{
					m_minHeight = qBound(4, minHeight.text().toInt(), 100);
				}
			}
		}

		// Section <Inputs>
		//
		{
			QDomElement inputsElement = xmlElement.firstChildElement(QLatin1String("Inputs"));

			if (inputsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Inputs. AFB %1").arg(m_strId);
				return false;
			}

			QDomElement p = inputsElement.firstChildElement(QLatin1String("Pin"));
			m_inputSignals.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbSignal afbSignal;

				bool ok = afbSignal.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_strId));
					return false;
				}

				m_inputSignals.push_back(afbSignal);

				p = p.nextSiblingElement(QLatin1String("Pin"));
			}
		}

		// Section <Outputs>
		//
		{
			QDomElement outputsElement = xmlElement.firstChildElement(QLatin1String("Outputs"));

			if (outputsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Outputs. AFB %1").arg(m_strId);
				return false;
			}

			QDomElement p = outputsElement.firstChildElement(QLatin1String("Pin"));
			m_outputSignals.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbSignal afbSignal;

				bool ok = afbSignal.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_strId));
					return false;
				}

				m_outputSignals.push_back(afbSignal);

				p = p.nextSiblingElement(QLatin1String("Pin"));
			}
		}

		// Section <Params>
		//
		{
			QDomElement paramsElement = xmlElement.firstChildElement(QLatin1String("Params"));

			if (paramsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Params. AFB %1").arg(m_strId);
				return false;
			}

			QDomElement p = paramsElement.firstChildElement(QLatin1String("Param"));
			m_params.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbParam afbParam;

				bool ok = afbParam.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_strId));
					return false;
				}

				m_params.push_back(afbParam);

				p = p.nextSiblingElement(QLatin1String("Param"));
			}
		}

		// Section <CommonScript>
		//
		m_libraryScript.clear();
		m_afterCreationScript.clear();

		{
			QDomElement commonScriptElement = xmlElement.firstChildElement(QLatin1String("CommonScript"));

			if (commonScriptElement.isNull() == false)
			{
				// Section <Library>
				//
				QDomElement libraryElement = commonScriptElement.firstChildElement(QLatin1String("Library"));

				if (libraryElement.isNull() == false)
				{
					m_libraryScript = libraryElement.text().trimmed();
				}

				// Section <AfterCreation>
				//
				QDomElement afterCreationElement = commonScriptElement.firstChildElement(QLatin1String("AfterCreation"));

				if (afterCreationElement.isNull() == false)
				{
					m_afterCreationScript = afterCreationElement.text().trimmed();
				}
			}
		}

		return true;
	}

	bool AfbElement::deprecatedFormatLoad(const Proto::AfbElementXml& data, QString& errorMsg)
	{
		QByteArray ba(data.data().data(), static_cast<int>(data.data().size()));
		QXmlStreamReader xmlReader(ba);

		// Read only StrID of element and PARAMS
		//
		if (xmlReader.readNextStartElement() == false)
		{
			return !xmlReader.hasError();
		}

		if (QString::compare(xmlReader.name().toString(), QLatin1String("ApplicationFunctionalBlocks"), Qt::CaseInsensitive) != 0)
		{
			errorMsg = QObject::tr("The file is not an ApplicationFunctionalBlocks file.");
			return !xmlReader.hasError();
		}

		if (xmlReader.readNextStartElement() == false)
		{
			return !xmlReader.hasError();
		}


		if (QString::compare(xmlReader.name().toString(), QLatin1String("AfbElement"), Qt::CaseInsensitive) != 0)
		{
			errorMsg = QObject::tr("AfbElement expected.");
			return !xmlReader.hasError();
		}

		if (xmlReader.attributes().hasAttribute(QLatin1String("StrId")) == true)
		{
			setStrID(xmlReader.attributes().value(QLatin1String("StrId")).toString());
		}

		// Reading params
		//
		std::vector<AfbParam> params;

		while (xmlReader.readNextStartElement())
		{
			if (QString::compare(xmlReader.name().toString(), QLatin1String("Properties"), Qt::CaseInsensitive) == 0)
			{
				while (xmlReader.readNextStartElement())
				{
					if (QString::compare(xmlReader.name().toString(), QLatin1String("Caption"), Qt::CaseInsensitive) == 0)
					{
						setCaption(xmlReader.readElementText());
						continue;
					}

					if (QString::compare(xmlReader.name().toString(), QLatin1String("Version"), Qt::CaseInsensitive) == 0)
					{
						setVersion(xmlReader.readElementText());
						continue;
					}

					if (QString::compare(xmlReader.name().toString(), QLatin1String("OpCode"), Qt::CaseInsensitive) == 0)
					{
						int opCode = xmlReader.readElementText().toInt();
						setOpCode(opCode);
						continue;
					}

					xmlReader.skipCurrentElement();
				}

				continue;
			}


			if (QString::compare(xmlReader.name().toString(), QLatin1String("Params"), Qt::CaseInsensitive) == 0)
			{
				// Read params
				//
				while (xmlReader.readNextStartElement())
				{
					if (QString::compare(xmlReader.name().toString(), QLatin1String("AfbElementParam"), Qt::CaseInsensitive) == 0)
					{
						AfbParam afbParam;
						afbParam.deprecatedLoadFromXml(&xmlReader);
						params.push_back(afbParam);
						xmlReader.skipCurrentElement();
					}
					else
					{
						errorMsg = QObject::tr("Unknown tag: ") + xmlReader.name().toString();
						xmlReader.skipCurrentElement();
					}
				}
			}

			xmlReader.skipCurrentElement();
		}

		setParams(params);

		return !xmlReader.hasError();
	}

	bool AfbElement::saveToXml(Proto::AfbElementXml* dst) const
	{
		QDomDocument d;
		QDomElement xmlSection = d.createElement(QLatin1String("AFB"));

		bool result = saveToXml(&xmlSection);

		if (result == true)
		{
			QByteArray ba;
			QTextStream stream(&ba);
			stream << xmlSection;

			dst->set_data(ba.data(), ba.size());
		}

		return result;
	}

	//	bool AfbElement::saveToXml(QByteArray* dst) const
	//	{
	//		QXmlStreamWriter writer(dst);
	//		bool result = saveToXml(&writer);
	//		return result;
	//	}

	bool AfbElement::saveToXml(QDomElement* xmlElement) const
	{
		if (xmlElement == nullptr ||
				xmlElement->isNull() == true)
		{
			assert(xmlElement);
			assert(xmlElement->isNull() == false);
			return false;
		}

		QDomDocument doc = xmlElement->ownerDocument();
		assert(doc.isNull() == false);

		xmlElement->setTagName(QLatin1String("AFB"));

		// id
		//
		xmlElement->setAttribute(QLatin1String("id"), m_strId);

		// Caption
		//
		xmlElement->setAttribute(QLatin1String("Caption"), m_caption);

		// Version
		//
		xmlElement->setAttribute(QLatin1String("Version"), m_version);

		// Section <Properties>
		//
		{
			QDomElement properies = doc.createElement(QLatin1String("Properties"));
			properies = xmlElement->appendChild(properies).toElement();

			// Section Properties::Description
			//
			{
				QDomElement s = doc.createElement(QLatin1String("Description"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_description));
			}

			// Section Properties::Category
			//
			{
				QDomElement s = doc.createElement(QLatin1String("Category"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_category));
			}

			// Section Properties::OpCode
			//
			{
				QDomElement s = doc.createElement(QLatin1String("OpCode"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(QString::number(opCode())));
			}

			// Section Properties::HasRam
			//
			{
				QDomElement s = doc.createElement(QLatin1String("HasRam"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_hasRam ? "true" : "false"));
			}

			// Section Properties::InternalUse
			//
			{
				QDomElement s = doc.createElement(QLatin1String("InternalUse"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_internalUse ? "true" : "false"));
			}

			// Section Properties::MinWidth
			//
			{
				QDomElement s = doc.createElement(QLatin1String("MinWidth"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(QString::number(minWidth())));
			}

			// Section Properties::MinHeight
			//
			{
				QDomElement s = doc.createElement(QLatin1String("MinHeight"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(QString::number(minHeight())));
			}
		}

		// Section <Inputs>
		//
		{
			QDomElement inputs = doc.createElement(QLatin1String("Inputs"));
			inputs = xmlElement->appendChild(inputs).toElement();

			for (const AfbSignal& pin : m_inputSignals)
			{
				QDomElement pinXml = doc.createElement(QLatin1String("Pin"));
				pinXml = inputs.appendChild(pinXml).toElement();

				pin.saveToXml(&pinXml);
			}
		}

		// Section <Outputs>
		//
		{
			QDomElement outputs = doc.createElement(QLatin1String("Outputs"));
			outputs = xmlElement->appendChild(outputs).toElement();

			for (const AfbSignal& pin : m_outputSignals)
			{
				QDomElement pinXml = doc.createElement(QLatin1String("Pin"));
				pinXml = outputs.appendChild(pinXml).toElement();

				pin.saveToXml(&pinXml);
			}
		}

		// Section <Params>
		//
		{
			QDomElement paramas = doc.createElement(QLatin1String("Params"));
			paramas = xmlElement->appendChild(paramas).toElement();

			for (const AfbParam& param : m_params)
			{
				QDomElement paramXml = doc.createElement(QLatin1String("Param"));
				paramXml = paramas.appendChild(paramXml).toElement();

				param.saveToXml(&paramXml);
			}
		}

		// Section <CommonScript>
		//
		{
			QDomElement commonScriptXml = doc.createElement(QLatin1String("CommonScript"));
			commonScriptXml = xmlElement->appendChild(commonScriptXml).toElement();

			// LibraryScript
			//
			{
				QDomElement s = doc.createElement(QLatin1String("Library"));
				s = commonScriptXml.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_libraryScript));
			}

			// AfterCreation
			//
			{
				QDomElement s = doc.createElement(QLatin1String("AfterCreation"));
				s = commonScriptXml.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_afterCreationScript));
			}
		}

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

	QObject* AfbElement::getAfbSignalByCaption(QString caption)
	{
		for (AfbSignal& s : m_inputSignals)
		{
			if (s.caption() == caption)
			{
				return &s;
			}
		}
		for (AfbSignal& s : m_outputSignals)
		{
			if (s.caption() == caption)
			{
				return &s;
			}
		}
		return nullptr;
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
				return p.opName() == mp.opName() && p.type() == mp.type();
			});

			if (found != m_params.end())
			{
				found->update(p.type(), p.dataFormat(), p.byteOrder(), p.lowLimit(), p.highLimit());
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
	const QString& AfbElement::strID() const
	{
		return m_strId;
	}
	void AfbElement::setStrID(const QString& strID)
	{
		m_strId = strID;
	}

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

	QString AfbElement::version() const
	{
		return m_version;
	}

	void AfbElement::setVersion(const QString& value)
	{
		m_version = value;
	}

	QString AfbElement::category() const
	{
		return m_category;
	}

	void AfbElement::setCategory(const QString& value)
	{
		m_category = value;
	}

	// Type - Opcode
	//
	int AfbElement::opCode() const
	{
		return m_opCode;
	}

	void AfbElement::setOpCode(int value)
	{
		m_opCode = value;
	}

	bool AfbElement::hasRam() const
	{
		return m_hasRam;
	}

	void AfbElement::setHasRam(bool value)
	{
		m_hasRam = value;
	}

	bool AfbElement::internalUse() const
	{
		return m_internalUse;
	}

	void AfbElement::setInternalUse(bool value)
	{
		m_internalUse = value;
	}

	int AfbElement::minWidth() const
	{
		return m_minWidth;
	}

	void AfbElement::setMinWidth(int value)
	{
		m_minWidth = qBound(10, value, 100);
	}

	int AfbElement::minHeight() const
	{
		return m_minHeight;
	}

	void AfbElement::setMinHeight(int value)
	{
		m_minHeight = qBound(4, value, 100);
	}

	QString AfbElement::libraryScript() const
	{
		return m_libraryScript;
	}

	void AfbElement::setLibraryScript(const QString& value)
	{
		m_libraryScript = value.trimmed();
	}

	QString AfbElement::afterCreationScript() const
	{
		return m_afterCreationScript;
	}

	void AfbElement::setAfterCreationScript(const QString& value)
	{
		m_afterCreationScript = value.trimmed();
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

	std::shared_ptr<Afb::AfbComponent> AfbElement::component()
	{
		assert(opCode() == m_component->opCode());
		return m_component;
	}

	std::shared_ptr<Afb::AfbComponent> AfbElement::component() const
	{
		assert(opCode() == m_component->opCode());
		return m_component;
	}

	void AfbElement::setComponent(std::shared_ptr<Afb::AfbComponent> value)
	{
		m_component = value;
		assert(opCode() == m_component->opCode());
	}

	QString AfbElement::componentCaption() const
	{
		if (m_component == nullptr)
		{
			assert(m_component);
			return QString();
		}

		return m_component->caption();
	}

	//
	//
	//		FblElementCollection
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

			QString errorMsg;
			bool result = e->loadFromXml(message.elements(i), &errorMsg);

			if (result == true)
			{
				m_elements.push_back(e);
			}
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

