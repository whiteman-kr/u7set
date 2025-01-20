#include <HardwareLib/Afb.h>
#include <Afb.pb.h>

namespace Afb
{

	AfbComponentPin::AfbComponentPin(const QString& caption, int opIndex, AfbComponentPinType type) :
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
			*errorMessage = "AFBComponent\\Pin does not have attribute OpName";
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("OpName"));

		// OpIndex -> m_opIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("OpIndex")) == false)
		{
			*errorMessage = "AFBComponent\\Pin does not have attribute OpIndex";
			return false;
		}

		m_opIndex = xmlElement.attribute(QLatin1String("OpIndex")).toInt();

		// PinType -> m_type
		//
		if (xmlElement.hasAttribute(QLatin1String("PinType")) == false)
		{
			*errorMessage = "AFBComponent\\Pin does not have attribute PinType";
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


	//
	//				AfbComponent
	//
	AfbComponent::AfbComponent()
	{
	}

	AfbComponent::AfbComponent(const AfbComponent& that)
	{
		m_opCode = that.m_opCode;
		m_hasRam = that.m_hasRam;
		m_caption = that.m_caption;
		m_impVersion = that.m_impVersion;
		m_versionOpIndex = that.m_versionOpIndex;
		m_maxInstCount = that.m_maxInstCount;
		m_simulationFunc = that.m_simulationFunc;

		m_pins = that.m_pins;
		m_pinExists = that.m_pinExists;

		return;
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
			*errorMessage = "AFBComponent does not have attribute Caption";
			return false;
		}

		m_caption = xmlElement.attribute(QLatin1String("Caption"));

		// OpCode
		//
		if (xmlElement.hasAttribute(QLatin1String("OpCode")) == false)
		{
			*errorMessage = QString("AFBComponent %1 does not have attribute OpCode").arg(m_caption);
			return false;
		}

		m_opCode = xmlElement.attribute(QLatin1String("OpCode")).toInt();

		// HasRam
		//
		if (xmlElement.hasAttribute(QLatin1String("HasRam")) == false)
		{
			*errorMessage = QString("AFBComponent %1 does not have attribute HasRam").arg(m_caption);
			return false;
		}

		m_hasRam = xmlElement.attribute(QLatin1String("HasRam")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0 ? true : false;

		// ImpVersion
		//
		if (xmlElement.hasAttribute(QLatin1String("ImpVersion")) == false)
		{
			*errorMessage = QString("AFBComponent %1 does not have attribute ImpVersion").arg(m_caption);
			return false;
		}

		m_impVersion = xmlElement.attribute(QLatin1String("ImpVersion")).toInt();

		// VersionOpIndex
		//
		if (xmlElement.hasAttribute(QLatin1String("VersionOpIndex")) == false)
		{
			*errorMessage = QString("AFBComponent %1 does not have attribute VersionOpIndex").arg(m_caption);
			return false;
		}

		m_versionOpIndex = xmlElement.attribute(QLatin1String("VersionOpIndex")).toInt();

		// MaxInstCount
		//
		if (xmlElement.hasAttribute(QLatin1String("MaxInstCount")) == false)
		{
			*errorMessage = QString("AFBComponent %1 does not have attribute MaxInstCount").arg(m_caption);
			return false;
		}

		m_maxInstCount = xmlElement.attribute(QLatin1String("MaxInstCount")).toInt();

		// SimulationFunc
		//
		m_simulationFunc = xmlElement.attribute(QLatin1String("SimulationFunc"));

		// SoftwareImplemented
		//
		m_softwareImplemented = false;

		if (xmlElement.hasAttribute(QLatin1String("SoftwareImplemented")) == true)
		{
			m_softwareImplemented = (xmlElement.attribute(QLatin1String("SoftwareImplemented")).
												toLower().trimmed() == "true");
		}

		// Pins
		//
		{
			QDomElement p = xmlElement.firstChildElement(QLatin1String("Pin"));
			m_pins.clear();
			m_pins.reserve(16);
			m_pinExists.clear();

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

				if (static_cast<size_t>(pin.opIndex()) >= m_pinExists.size())
				{
					m_pinExists.resize(pin.opIndex() + 1);
				}

				m_pinExists[pin.opIndex()] = true;

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

	int AfbComponent::opCode() const noexcept
	{
		return m_opCode;
	}

	void AfbComponent::setOpCode(int value) noexcept
	{
		m_opCode = value;
	}

	bool AfbComponent::hasRam() const noexcept
	{
		return m_hasRam;
	}

	void AfbComponent::setHasRam(bool value) noexcept
	{
		m_hasRam = value;
	}

	const QString& AfbComponent::caption() const noexcept
	{
		return m_caption;
	}

	void AfbComponent::setCaption(const QString& value) noexcept
	{
		m_caption = value;
	}

	int AfbComponent::impVersion() const noexcept
	{
		return m_impVersion;
	}

	void AfbComponent::setImpVersion(int value) noexcept
	{
		m_impVersion = value;
	}

	int AfbComponent::versionOpIndex() const noexcept
	{
		return m_versionOpIndex;
	}

	void AfbComponent::setVersionOpIndex(int value) noexcept
	{
		m_versionOpIndex = value;
	}

	int AfbComponent::maxInstCount() const noexcept
	{
		return m_maxInstCount;
	}

	void AfbComponent::setMaxInstCount(int value) noexcept
	{
		m_maxInstCount = value;
	}

	const QString& AfbComponent::simulationFunc() const noexcept
	{
		return m_simulationFunc;
	}

	void AfbComponent::setSimulationFunc(const QString& value) noexcept
	{
		m_simulationFunc = value;
	}

	bool AfbComponent::isSoftwareImplemented() const noexcept
	{
		return m_softwareImplemented;
	}

	void AfbComponent::setSoftwareImplemented(bool value) noexcept
	{
		m_softwareImplemented = value;
	}

	const std::unordered_map<int, AfbComponentPin>& AfbComponent::pins() const noexcept
	{
		return m_pins;
	}

	bool AfbComponent::pinExists(int pinOpIndex) const noexcept
	{
		return (static_cast<size_t>(pinOpIndex) >= m_pinExists.size())
				? false: m_pinExists[pinOpIndex];
	}

	QString AfbComponent::pinCaption(int pinOpIndex) const noexcept
	{
		auto it = m_pins.find(pinOpIndex);
		if (it != m_pins.end())
		{
			return it->second.caption();
		}

		return QLatin1String("[UnknownPin ") + QString::number(pinOpIndex) + QLatin1String("]");
	}

	int AfbComponent::pinOpIndex(const QString& pinCaption) const noexcept
	{
		for(auto& pinInfo : m_pins)
		{
			if (pinInfo.second.caption() == pinCaption)
			{
				return pinInfo.first;	// pin opIndex
			}
		}

		return -1;
	}

	//
	//							AfbSignal
	//
	AfbSignal::AfbSignal(void)
	{
	}

	AfbSignal::AfbSignal(const AfbSignal& that) : QObject(that.parent())
	{
		*this = that;
	}

	AfbSignal::AfbSignal(AfbSignal&& that) noexcept :
		QObject(that.parent()),
		m_opName(std::move(that.m_opName)),
		m_caption(std::move(that.m_caption)),
		m_type(that.m_type),
		m_dataFormat(that.m_dataFormat),
		m_operandIndex(that.m_operandIndex),
		m_size(that.m_size),
		m_additionalSizes(std::move(that.m_additionalSizes)),
		m_byteOrder(that.m_byteOrder),
		m_busDataFormat(that.m_busDataFormat)
	{
	}

	AfbSignal::~AfbSignal()
	{
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
		m_additionalSizes = that.m_additionalSizes;
		m_byteOrder = that.m_byteOrder;
		m_busDataFormat = that.m_busDataFormat;

		return *this;
	}

	AfbSignal& AfbSignal::operator=(AfbSignal&& that) noexcept
	{
		if (this == &that)
		{
			return *this;
		}

		m_opName = std::move(that.m_opName);
		m_caption = std::move(that.m_caption);
		m_type = that.m_type;
		m_dataFormat = that.m_dataFormat;
		m_operandIndex = that.m_operandIndex;
		m_size = that.m_size;
		m_additionalSizes = std::move(that.m_additionalSizes);
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

		m_size = xmlElement.attribute(QLatin1String("Size")).toInt();

		// AdditionalSizes
		//
		if (xmlElement.hasAttribute(QLatin1String("AdditionalSizes")) == true)
		{
			QStringList str = xmlElement.attribute(QLatin1String("AdditionalSizes"))
			              .split(",", Qt::SkipEmptyParts);

			m_additionalSizes.clear();
			m_additionalSizes.reserve(str.size());

			for (auto& s : str)
			{
				bool ok = false;
				uint as = s.trimmed().toUInt(&ok);

				if (ok == true)
				{
					m_additionalSizes.push_back(as);
				}
			}
		}

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

		// AdditionalSizes
		//
		if (m_additionalSizes.empty() == false)
		{
			QString str;
			str.reserve(m_additionalSizes.size() * 4);

			for (size_t i = 0; i < m_additionalSizes.size(); i++)
			{
				if (i == m_additionalSizes.size() - 1)
				{
					str += QString::number(m_additionalSizes[i]);
				}
				else
				{
					str += QString("%d, ").arg(m_additionalSizes[i]);
				}
			}

			element->setAttribute(QLatin1String("AdditionalSizes"), str);
		}

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
	QString AfbSignal::caption() const
	{
		return m_caption;
	}

	QString AfbSignal::jsCaption()
	{
		return caption();
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

	const std::vector<int>& AfbSignal::additionalSizes() const
	{
		return m_additionalSizes;
	}

	void AfbSignal::setAdditionalSizes(std::vector<int> value)
	{
		m_additionalSizes = std::move(value);
	}

	std::vector<int> AfbSignal::allSizes() const
	{
		// Returns size() and additionalSizes() as single vector
		//
		std::vector<int> result;
		result.reserve(m_additionalSizes.size() + 1);

		result.push_back(m_size);
		std::copy(m_additionalSizes.begin(), m_additionalSizes.end(), std::back_inserter(result));

		return result;
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
	//							AfbParam
	//
	AfbParam::AfbParam(void) :
		m_visible(true),
		m_byteOrder(E::ByteOrder::BigEndian),
		m_instantiator(false),
		m_user(false),
		m_afbParamValue(E::SignalType::Analog, E::DataFormat::UnsignedInt, 16),
		m_operandIndex(0)
	{
		m_defaultValue = 0;
		m_lowLimit = 0;
		m_highLimit = 0;
	}

	void AfbParam::update(const E::SignalType& type, const E::DataFormat dataFormat, E::ByteOrder byteOrder, const QVariant &lowLimit, const QVariant &highLimit)
	{
		m_afbParamValue.setType(type);
		m_afbParamValue.setDataFormat(dataFormat);
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

		if (xmlReader->name() != QLatin1String("AfbElementParam"))
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
						afbParamValue().setValue(val);
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

		int size = xmlElement.attribute(QLatin1String("Size")).toInt();
		afbParamValue().setSize(size);

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
			m_afbParamValue.setType(E::SignalType::Analog);

			// Analog signal size cannot be less then 16 bits
			// Fix size, in old versions of LM descriptions the size og AfbParam could be 4 or 5 (i_conf for example),
			// but in reality size of param is 16bit, so fix this problem for stored AFB params here
			// Also puls_gen had an error, discrete signal was saved as 1-bit analog
			//
			if (m_afbParamValue.size() < 16)
			{
				m_afbParamValue.setSize(16);
			}
		}
		else
		{
			if (typeAttribute.compare(QLatin1String(QLatin1String("Discrete")), Qt::CaseInsensitive) == 0)
			{
				m_afbParamValue.setType(E::SignalType::Discrete);
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
				m_afbParamValue.setDataFormat(E::DataFormat::UnsignedInt);
			}
			else
			{
				if (dataFormatAttribute.compare(QLatin1String("SignedInt"), Qt::CaseInsensitive) == 0)
				{
					m_afbParamValue.setDataFormat(E::DataFormat::SignedInt);
				}
				else
				{
					if (dataFormatAttribute.compare(QLatin1String("Float"), Qt::CaseInsensitive) == 0)
					{
						m_afbParamValue.setDataFormat(E::DataFormat::Float);
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
				[](const QString& str, E::SignalType type, E::DataFormat dataFormat) -> QVariant
				{
					if (type == E::SignalType::Analog)
					{
						switch (dataFormat)
						{
						case E::DataFormat::UnsignedInt:
						case E::DataFormat::SignedInt:
							return {str.toInt()};
						case E::DataFormat::Float:
							return {str.toDouble()};
						default:
							assert(false);
							return {};
						}
					}

					if (type == E::SignalType::Discrete)
					{
						return {str == "1" ? true : false};
					}

					assert(false);
					return {};
				};

		{
			QDomElement valueElement = xmlElement.firstChildElement(QLatin1String("Value"));

			if (valueElement.isNull() == true)
			{
				*errorMessage = QString("Can't find section Value. Param %1.").arg(m_caption);
				return false;
			}

			QVariant v = valToDataFormat(valueElement.text(), type(), dataFormat());
			afbParamValue().setValue(v);
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

		// Section <ValueReference>
		// Value reference string (AfbParamValue::reference())
		// May not exist
		//
		{
			QDomElement e = xmlElement.firstChildElement(QLatin1String("ValueReference"));
			m_afbParamValue.setReference(e.isNull() ? QString{} : e.text());
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
		xmlElement->setAttribute(QLatin1String("Size"), afbParamValue().size());

		// Instantiator
		//
		xmlElement->setAttribute(QLatin1String("Instantiator"), m_instantiator ? QLatin1String("true") : QLatin1String("false"));

		// User
		//
		xmlElement->setAttribute(QLatin1String("User"), m_user ? QLatin1String("true") : QLatin1String("false"));

		// Type
		//
		xmlElement->setAttribute(QLatin1String("Type"), E::valueToString(m_afbParamValue.type()));

		// DataFormat
		//
		xmlElement->setAttribute(QLatin1String("DataFormat"), E::valueToString(m_afbParamValue.dataFormat()));

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
				QDomText text = doc.createTextNode(afbParamValue().value() == true ? QLatin1String("1") : QLatin1String("0"));
				node.appendChild(text);
			}
			// Default
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Default")));
				QDomText text = doc.createTextNode(defaultValue() == true ? QLatin1String("1") : QLatin1String("0"));
				node.appendChild(text);
			}
			// LowLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("LowLimit")));
				QDomText text = doc.createTextNode(lowLimit() == true ? QLatin1String("1") : QLatin1String("0"));
				node.appendChild(text);
			}
			// HighLimit
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("HighLimit")));
				QDomText text = doc.createTextNode(highLimit() == true ? QLatin1String("1") : QLatin1String("0"));
				node.appendChild(text);
			}
		}
		else
		{
			// Value
			{
				QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("Value")));
				QDomText text = doc.createTextNode(afbParamValue().value().toString());
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

		// Section <ValueReference>
		// Value reference string (AfbParamValue::reference())
		// May not exist
		//
		if (m_afbParamValue.reference().isEmpty() == false)
		{
			QDomNode node = xmlElement->appendChild(doc.createElement(QLatin1String("ValueReference")));
			QDomText text = doc.createTextNode(m_afbParamValue.reference());
			node.appendChild(text);
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
		return m_afbParamValue.type();
	}
	void AfbParam::setType(E::SignalType type)
	{
		m_afbParamValue.setType(type);
	}

	E::DataFormat AfbParam::dataFormat() const
	{
		return m_afbParamValue.dataFormat();
	}

	void AfbParam::setDataFormat(E::DataFormat dataFormat)
	{
		m_afbParamValue.setDataFormat(dataFormat);
	}

	bool AfbParam::isValid() const
	{
		return	!m_caption.isEmpty() &&
				!m_opName.isEmpty() &&
				m_operandIndex >= 0;
	}

	bool AfbParam::isAnalog() const
	{
		return m_afbParamValue.type() == E::SignalType::Analog;
	}

	bool AfbParam::isDiscrete() const
	{
		return m_afbParamValue.type() == E::SignalType::Discrete;
	}

	// Value
	//
	const AfbParamValue& AfbParam::afbParamValue() const
	{
		return m_afbParamValue;
	}

	AfbParamValue& AfbParam::afbParamValue()
	{
		return m_afbParamValue;
	}

	void AfbParam::setAfbParamValue(const AfbParamValue& v)
	{
		m_afbParamValue = v;
	}

	// Default Value
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
		return afbParamValue().size();
	}

	void AfbParam::setSize(int value)
	{
		afbParamValue().setSize(value);
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

	void AfbParam::setChangedScript(const QString& value)
	{
		m_changedScript = value.trimmed();
	}

	const QString& AfbParam::units() const
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

		m_data = that.m_data;

		return *this;
	}

	bool AfbElement::loadFromXml(const Proto::AfbElementXml& data, QString* errorMsg)
	{
		QByteArray ba = QByteArray::fromRawData(data.data().data(), static_cast<int>(data.data().size()));

		QDomDocument doc;

		QDomDocument::ParseResult pr = doc.setContent(ba);

		if (pr.errorMessage.isEmpty() == false)
		{
			return false;
		}

		bool result = loadFromXml(doc.firstChild().toElement(), errorMsg);

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

		m_data.strId = xmlElement.attribute(QLatin1String("id"));

		// Caption
		//
		if (xmlElement.hasAttribute(QLatin1String("Caption")) == false)
		{
			*errorMessage = tr("Cant find attribute Caption. AFB %1").arg(m_data.strId);
			return false;
		}

		m_data.caption = xmlElement.attribute(QLatin1String("Caption"));

		// Version
		//
		if (xmlElement.hasAttribute(QLatin1String("Version")) == false)
		{
			*errorMessage = tr("Cant find attribute Version. AFB %1").arg(m_data.strId);
			return false;
		}

		m_data.version = xmlElement.attribute(QLatin1String("Version"));

		// Section Properties
		//
		{
			QDomElement properties = xmlElement.firstChildElement(QLatin1String("Properties"));

			if (properties.isNull() == true)
			{
				*errorMessage = tr("Cant find section Properties. AFB %1").arg(m_data.strId);
				return false;
			}

			// Section Properties::Description
			//
			{
				QDomElement description = properties.firstChildElement(QLatin1String("Description"));

				if (description.isNull() == true)
				{
					*errorMessage = tr("Cant find section Description. AFB %1").arg(m_data.strId);
					return false;
				}

				m_data.description = description.text();
			}

			// Section Properties::Category
			//
			{
				QDomElement category = properties.firstChildElement(QLatin1String("Category"));

				if (category.isNull() == true)
				{
					*errorMessage = tr("Cant find section Category. AFB %1").arg(m_data.strId);
					return false;
				}

				m_data.category = category.text();
			}

			// Section Properties::OpCode
			//
			{
				QDomElement opCode = properties.firstChildElement(QLatin1String("OpCode"));

				if (opCode.isNull() == true)
				{
					*errorMessage = tr("Cant find section OpCode. AFB %1").arg(m_data.strId);
					return false;
				}

				m_data.opCode = opCode.text().toInt();
			}
			//m_type.fromOpCode(m_opCode);

			// Section Properties::HasRam is optional
			//
			{
				QDomElement hasRam = properties.firstChildElement(QLatin1String("HasRam"));

				if (hasRam.isNull() == false)
				{
					m_data.hasRam = hasRam.text().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
				}
			}

			// Section Properties::InternalUse
			//
			{
				QDomElement internalUse = properties.firstChildElement(QLatin1String("InternalUse"));

				if (internalUse.isNull() == true)
				{
					*errorMessage = tr("Cant find section InternalUse. AFB %1").arg(m_data.strId);
					return false;
				}

				m_data.internalUse = internalUse.text().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
			}

			// Section Properties::MinWidth
			//
			{
				QDomElement minWidth = properties.firstChildElement(QLatin1String("MinWidth"));

				if (minWidth.isNull() == true)
				{
					m_data.minWidth = 10;
				}
				else
				{
					m_data.minWidth = qBound(10, minWidth.text().toInt(), 100);
				}
			}

			// Section Properties::MinHeight
			//
			{
				QDomElement minHeight = properties.firstChildElement(QLatin1String("MinHeight"));

				if (minHeight.isNull() == true)
				{
					m_data.minHeight = 4;
				}
				else
				{
					m_data.minHeight = qBound(4, minHeight.text().toInt(), 100);
				}
			}

			// Section <PackedLogic Counterpart="packed_or_out" IdPrefix="POR_" MinInputCount="1"/>
			//
			{
				QDomElement packedLogic = properties.firstChildElement(QLatin1String("PackedLogic"));

				if (packedLogic.isNull() == false)
				{
					m_data.packedLogic.counterpart = packedLogic.attribute(QLatin1String("Counterpart"));
					m_data.packedLogic.idPrefix = packedLogic.attribute(QLatin1String("IdPrefix"));
					m_data.packedLogic.minInputCount = packedLogic.attribute(QLatin1String("MinInputCount"), "99999").toInt();
					
					// Do not update item if attribute does not exist, this is update of the AFB ite, we need to keep value.
					// Update: Actually value is preserved in SchemaItemAfb.
					//
					m_data.packedLogicId = packedLogic.attribute(QLatin1String("packedLogicId"), m_data.packedLogicId);
				}
			}
		}

		// Section <Inputs>
		//
		{
			QDomElement inputsElement = xmlElement.firstChildElement(QLatin1String("Inputs"));

			if (inputsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Inputs. AFB %1").arg(m_data.strId);
				return false;
			}

			QDomElement p = inputsElement.firstChildElement(QLatin1String("Pin"));
			m_data.inputSignals.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbSignal afbSignal;

				bool ok = afbSignal.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_data.strId));
					return false;
				}

				m_data.inputSignals.push_back(afbSignal);

				p = p.nextSiblingElement(QLatin1String("Pin"));
			}
		}

		// Section <Outputs>
		//
		{
			QDomElement outputsElement = xmlElement.firstChildElement(QLatin1String("Outputs"));

			if (outputsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Outputs. AFB %1").arg(m_data.strId);
				return false;
			}

			QDomElement p = outputsElement.firstChildElement(QLatin1String("Pin"));
			m_data.outputSignals.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbSignal afbSignal;

				bool ok = afbSignal.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_data.strId));
					return false;
				}

				m_data.outputSignals.push_back(afbSignal);

				p = p.nextSiblingElement(QLatin1String("Pin"));
			}
		}

		// Section <Params>
		//
		{
			QDomElement paramsElement = xmlElement.firstChildElement(QLatin1String("Params"));

			if (paramsElement.isNull() == true)
			{
				*errorMessage = tr("Cant find section Params. AFB %1").arg(m_data.strId);
				return false;
			}

			QDomElement p = paramsElement.firstChildElement(QLatin1String("Param"));
			m_data.params.clear();

			while (p.isNull() == false)
			{
				// p is Pin section
				//
				AfbParam afbParam;

				bool ok = afbParam.loadFromXml(p, errorMessage);
				if (ok == false)
				{
					errorMessage->append(tr(" AFB %1").arg(m_data.strId));
					return false;
				}

				m_data.params.push_back(afbParam);

				p = p.nextSiblingElement(QLatin1String("Param"));
			}
		}

		// Section <CommonScript>
		//
		m_data.libraryScript.clear();
		m_data.afterCreationScript.clear();

		{
			QDomElement commonScriptElement = xmlElement.firstChildElement(QLatin1String("CommonScript"));

			if (commonScriptElement.isNull() == false)
			{
				// Section <Library>
				//
				QDomElement libraryElement = commonScriptElement.firstChildElement(QLatin1String("Library"));

				if (libraryElement.isNull() == false)
				{
					m_data.libraryScript = libraryElement.text().trimmed();
				}

				// Section <AfterCreation>
				//
				QDomElement afterCreationElement = commonScriptElement.firstChildElement(QLatin1String("AfterCreation"));

				if (afterCreationElement.isNull() == false)
				{
					m_data.afterCreationScript = afterCreationElement.text().trimmed();
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
		xmlElement->setAttribute(QLatin1String("id"), m_data.strId);

		// Caption
		//
		xmlElement->setAttribute(QLatin1String("Caption"), m_data.caption);

		// Version
		//
		xmlElement->setAttribute(QLatin1String("Version"), m_data.version);

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
				s.appendChild(doc.createTextNode(m_data.description));
			}

			// Section Properties::Category
			//
			{
				QDomElement s = doc.createElement(QLatin1String("Category"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_data.category));
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
			if (m_data.hasRam.has_value() == true)
			{
				QDomElement s = doc.createElement(QLatin1String("HasRam"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_data.hasRam.value() ? "true" : "false"));
			}

			// Section Properties::InternalUse
			//
			{
				QDomElement s = doc.createElement(QLatin1String("InternalUse"));
				s = properies.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_data.internalUse ? "true" : "false"));
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

			// Section <PackedLogic Function = "or" Counterpart = "packed_or_out" IdPrefix = "POR_" MinInputCount="1"/ >
			//
			{
				QDomElement s = doc.createElement(QLatin1String("PackedLogic"));
				s = properies.appendChild(s).toElement();
				s.setAttribute(QLatin1String("Counterpart"), m_data.packedLogic.counterpart);
				s.setAttribute(QLatin1String("IdPrefix"), m_data.packedLogic.idPrefix);
				s.setAttribute(QLatin1String("MinInputCount"), m_data.packedLogic.minInputCount);

				s.setAttribute(QLatin1String("packedLogicId"), m_data.packedLogicId);
			}
		}

		// Section <Inputs>
		//
		{
			QDomElement inputs = doc.createElement(QLatin1String("Inputs"));
			inputs = xmlElement->appendChild(inputs).toElement();

			for (const AfbSignal& pin : m_data.inputSignals)
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

			for (const AfbSignal& pin : m_data.outputSignals)
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

			for (const AfbParam& param : m_data.params)
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
				s.appendChild(doc.createTextNode(m_data.libraryScript));
			}

			// AfterCreation
			//
			{
				QDomElement s = doc.createElement(QLatin1String("AfterCreation"));
				s = commonScriptXml.appendChild(s).toElement();
				s.appendChild(doc.createTextNode(m_data.afterCreationScript));
			}
		}

		return true;
	}

	QObject* AfbElement::getAfbSignalByOpIndex(int opIndex)
	{
		for (AfbSignal& s : m_data.inputSignals)
		{
			if (s.operandIndex() == opIndex)
			{
				return &s;
			}
		}

		for (AfbSignal& s : m_data.outputSignals)
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
		for (AfbSignal& s : m_data.inputSignals)
		{
			if (s.caption() == caption)
			{
				return &s;
			}
		}

		for (AfbSignal& s : m_data.outputSignals)
		{
			if (s.caption() == caption)
			{
				return &s;
			}
		}

		return nullptr;
	}

	// Properties and Data
	//
	const QString& AfbElement::strID() const
	{
		return m_data.strId;
	}
	void AfbElement::setStrID(const QString& strID)
	{
		m_data.strId = strID;
	}

	const QString& AfbElement::caption() const
	{
		return m_data.caption;
	}
	void AfbElement::setCaption(const QString& caption)
	{
		m_data.caption = caption;
	}

	const QString& AfbElement::description() const
	{
		return m_data.description;
	}

	void AfbElement::setDescription(const QString& value)
	{
		m_data.description = value;
	}

	const QString& AfbElement::version() const
	{
		return m_data.version;
	}

	void AfbElement::setVersion(const QString& value)
	{
		m_data.version = value;
	}

	const QString& AfbElement::category() const
	{
		return m_data.category;
	}

	void AfbElement::setCategory(const QString& value)
	{
		m_data.category = value;
	}

	// Type - Opcode
	//
	int AfbElement::opCode() const
	{
		return m_data.opCode;
	}

	void AfbElement::setOpCode(int value)
	{
		m_data.opCode = value;
	}

	std::optional<bool> AfbElement::hasRam() const
	{
		return m_data.hasRam;
	}

	void AfbElement::setHasRam(bool value)
	{
		m_data.hasRam = value;
	}

	bool AfbElement::internalUse() const
	{
		return m_data.internalUse;
	}

	void AfbElement::setInternalUse(bool value)
	{
		m_data.internalUse = value;
	}

	int AfbElement::minWidth() const
	{
		return m_data.minWidth;
	}

	void AfbElement::setMinWidth(int value)
	{
		m_data.minWidth = qBound(10, value, 100);
	}

	int AfbElement::minHeight() const
	{
		return m_data.minHeight;
	}

	void AfbElement::setMinHeight(int value)
	{
		m_data.minHeight = qBound(4, value, 100);
	}

	QString AfbElement::libraryScript() const
	{
		return m_data.libraryScript;
	}

	void AfbElement::setLibraryScript(const QString& value)
	{
		m_data.libraryScript = value.trimmed();
	}

	QString AfbElement::afterCreationScript() const
	{
		return m_data.afterCreationScript;
	}

	void AfbElement::setAfterCreationScript(const QString& value)
	{
		m_data.afterCreationScript = value.trimmed();
	}

	// InputSignals
	//
	const std::vector<AfbSignal>& AfbElement::inputSignals() const
	{
		return m_data.inputSignals;
	}
	void AfbElement::setInputSignals(const std::vector<AfbSignal>& inputsignals)
	{
		m_data.inputSignals = inputsignals;
	}

	// OutputSignals
	//
	const std::vector<AfbSignal>& AfbElement::outputSignals() const
	{
		return m_data.outputSignals;
	}
	void AfbElement::setOutputSignals(const std::vector<AfbSignal>& outputsignals)
	{
		m_data.outputSignals = outputsignals;
	}

	// Params
	//
	const std::vector<AfbParam>& AfbElement::params() const
	{
		return m_data.params;
	}

	std::vector<AfbParam>& AfbElement::params()
	{
		return m_data.params;
	}

	AfbParam AfbElement::paramByCaption(const QString& caption) const
	{
		for(const AfbParam& p : m_data.params)
		{
			if (p.caption() == caption)
			{
				return p;
			}
		}

		return AfbParam();
	}

	AfbParam AfbElement::paramByOpName(const QString& opName) const
	{
		for(const AfbParam& p : m_data.params)
		{
			if (p.opName() == opName)
			{
				return p;
			}
		}

		return AfbParam();
	}

	int AfbElement::paramsCount() const
	{
		return static_cast<int>(m_data.params.size());
	}

	void AfbElement::setParams(const std::vector<AfbParam>& params)
	{
		m_data.params = params;
	}

	std::shared_ptr<Afb::AfbComponent> AfbElement::component()
	{
		assert(opCode() == m_data.component->opCode());
		return m_data.component;
	}

	std::shared_ptr<Afb::AfbComponent> AfbElement::component() const
	{
		assert(opCode() == m_data.component->opCode());
		return m_data.component;
	}

	void AfbElement::setComponent(std::shared_ptr<Afb::AfbComponent> value)
	{
		m_data.component = value;
		assert(opCode() == m_data.component->opCode());
	}

	QString AfbElement::componentCaption() const
	{
		if (m_data.component == nullptr)
		{
			assert(m_data.component);
			return QString();
		}

		return m_data.component->caption();
	}

	bool AfbElement::isPackedLogic() const
	{
		return m_data.packedLogic.counterpart.isEmpty() == false;
	}

	const AfbElement::PackedLogicData& AfbElement::packedLogic() const
	{
		return m_data.packedLogic;
	}

	const QString& AfbElement::packedLogicId() const
	{
		return m_data.packedLogicId;
	}

	void AfbElement::setPackedLogicId(const QString& value)
	{
		m_data.packedLogicId = value;
	}

	//
	//
	//		FblElementCollection
	//
	//

	AfbElementCollection::AfbElementCollection(void)
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

