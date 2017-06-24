#include "Bus.h"
#include "PropertyNames.h"
#include <QDomDocument>

namespace VFrame30
{
	//
	// BusSignal
	//
	BusSignal::BusSignal() :
		BusSignal(E::SignalType::Discrete)
	{
	}

	BusSignal::BusSignal(const BusSignal& src)
	{
		*this = src;
	}

	BusSignal::BusSignal(E::SignalType type) :
		m_type(type)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::name, true, BusSignal::name, BusSignal::setName);
		ADD_PROPERTY_GETTER(E::SignalType, PropertyNames::type, true, BusSignal::type);

		switch (type)
		{
		case E::SignalType::Analog:
			ADD_PROPERTY_GETTER_SETTER(E::AnalogAppSignalFormat, PropertyNames::analogFormat, true, BusSignal::analogFormat, BusSignal::setAnalogFormat);
			break;
		case E::SignalType::Discrete:
			break;
		case E::SignalType::Bus:
			ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::busTypeId, true, BusSignal::busTypeId, BusSignal::setBusTypeId);
			break;
		default:
			assert(false);
		}

		return;
	}

	bool BusSignal::load(const QDomElement& domElement, QString* errorMessage)
	{
		if (domElement.isNull() == true ||
			errorMessage == nullptr)
		{
			assert(domElement.isNull() == false);
			assert(errorMessage != nullptr);
			return false;
		}

		// Name
		//
		if (domElement.hasAttribute(QLatin1String("Name")) == false)
		{
			*errorMessage += "Cant find attribute Name in BusSignal";
			return false;
		}

		m_name = domElement.attribute(QLatin1String("Name"));

		// Type
		//
		{
			if (domElement.hasAttribute(QLatin1String("Type")) == false)
			{
				*errorMessage += "Cant find attribute Type in BusSignal";
				return false;
			}

			QString strType = domElement.attribute(QLatin1String("Type"));

			if (strType.compare(QLatin1String("Analog"), Qt::CaseInsensitive) == 0)
			{
				m_type = E::SignalType::Analog;
			}
			else
			{
				if (strType.compare(QLatin1String("Discrete"), Qt::CaseInsensitive) == 0)
				{
					m_type = E::SignalType::Discrete;
				}
				else
				{
					if (strType.compare(QLatin1String("Bus"), Qt::CaseInsensitive) == 0)
					{
						m_type = E::SignalType::Bus;
					}
					else
					{
						*errorMessage += "Attribute Type has wrong value: " + strType;
						return false;
					}
				}
			}
		}

		// AnalogFormat
		//
		if (m_type == E::SignalType::Analog)
		{
			if (domElement.hasAttribute(QLatin1String("AnalogFormat")) == false)
			{
				*errorMessage += "Cant find attribute AnalogFormat in BusSignal";
				return false;
			}

			QString strAnalogFormat = domElement.attribute(QLatin1String("AnalogFormat"));

			if (strAnalogFormat.compare(QLatin1String("Float32"), Qt::CaseInsensitive) == 0)
			{
				m_analogDataFormat = E::AnalogAppSignalFormat::Float32;
			}
			else
			{
				if (strAnalogFormat.compare(QLatin1String("SignedInt32"), Qt::CaseInsensitive) == 0)
				{
					m_analogDataFormat = E::AnalogAppSignalFormat::SignedInt32;
				}
				else
				{
					*errorMessage += "Attribute AnalogFormat has wrong value: " + strAnalogFormat;
					return false;
				}
			}
		}

		// BusTypeID
		//
		if (m_type == E::SignalType::Bus)
		{
			if (domElement.hasAttribute(QLatin1String("BusTypeID")) == false)
			{
				*errorMessage += "Cant find attribute BusTypeID in BusSignal";
				return false;
			}

			m_busTypeId = domElement.attribute(QLatin1String("BusTypeID"));
		}

		return true;
	}

	bool BusSignal::save(QDomElement* domElement) const
	{
		if (domElement == nullptr)
		{
			assert(domElement);
			return false;
		}

		domElement->setTagName(QLatin1String("BusSignal"));

		// Name
		//
		domElement->setAttribute(QLatin1String("Name"), m_name);

		// Type
		//
		domElement->setAttribute(QLatin1String("Type"), E::valueToString(m_type));

		switch (m_type)
		{
		case E::SignalType::Discrete:
			break;
		case E::SignalType::Analog:
			domElement->setAttribute(QLatin1String("AnalogFormat"), E::valueToString(m_analogDataFormat));
			break;
		case E::SignalType::Bus:
			domElement->setAttribute(QLatin1String("BusTypeID"), m_busTypeId);
			break;
		default:
			assert(false);
		}

		return true;
	}

	QString BusSignal::name() const
	{
		return m_name;
	}

	void BusSignal::setName(const QString& value)
	{
		m_name = value.trimmed();
	}

	E::SignalType BusSignal::type() const
	{
		return m_type;
	}

	void BusSignal::setType(E::SignalType value)
	{
		m_type = value;
	}

	E::AnalogAppSignalFormat BusSignal::analogFormat() const
	{
		return m_analogDataFormat;
	}

	void BusSignal::setAnalogFormat(E::AnalogAppSignalFormat value)
	{
		m_analogDataFormat = value;
	}

	QString BusSignal::busTypeId() const
	{
		return m_busTypeId;
	}

	void BusSignal::setBusTypeId(const QString& value)
	{
		m_busTypeId = value.trimmed();
	}


	//
	// Bus
	//
	Bus::Bus()
	{
	}

	bool Bus::load(const QByteArray& data, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			return false;
		}

		errorMessage->clear();

		// Parse doc
		//
		QDomDocument doc;
		int parseErrorLine = -1;
		int parseErrorColumn = -1;

		bool ok = doc.setContent(data, errorMessage, &parseErrorLine, &parseErrorColumn);
		if (ok == false)
		{
			errorMessage->append(QString(" Error line %1, column %2").arg(parseErrorLine).arg(parseErrorColumn));
			return false;
		}

		// Read root element <BusType>
		//
		QDomElement busType = doc.documentElement();

		if (busType.isNull() == true ||
			busType.tagName() != "BusType")
		{
			*errorMessage = QString("Can't find section BusType.");
			return false;
		}

		if (busType.hasAttribute(QLatin1String("ID")) == false)
		{
			*errorMessage = QString("Can't find attribute ID in section BusType.");
			return false;
		}

		m_busTypeId = busType.attribute(QLatin1String("ID"));

		// Read set of <BusSignal>
		//
		{
			QDomNodeList busSignalList = busType.elementsByTagName(QLatin1String("BusSignal"));

			m_busSignals.clear();
			m_busSignals.reserve(busSignalList.size());

			for (int i = 0; i < busSignalList.size(); ++i)
			{
				QDomElement busSignalElement = busSignalList.at(i).toElement();

				BusSignal busSignal;
				ok = busSignal.load(busSignalElement, errorMessage);

				if (ok == false)
				{
					errorMessage->append(QString(" Error parsing %1 bus type").arg(m_busTypeId));
					return false;
				}

				m_busSignals.push_back(busSignal);
			}
		}

		return true;
	}

	bool Bus::save(QByteArray* data) const
	{
		if (data == nullptr)
		{
			assert(data);
			return false;
		}

		QDomDocument doc;

		QDomElement busTypeElement = doc.createElement(QLatin1String("BusType"));
		doc.appendChild(busTypeElement);

		busTypeElement.setAttribute(QLatin1String("ID"), m_busTypeId);

		for (const BusSignal& busSignal : m_busSignals)
		{
			QDomElement busSignalElement = doc.createElement(QLatin1String("BusSignal"));
			busTypeElement.appendChild(busSignalElement);

			busSignal.save(&busSignalElement);
		}

		*data = doc.toByteArray();
		return true;
	}

	QString Bus::busTypeId() const
	{
		return m_busTypeId;
	}

	void Bus::setBusTypeId(const QString& value)
	{
		m_busTypeId = value.trimmed();
	}

	Hash Bus::calcHash() const
	{
		QByteArray data;
		bool ok = save(&data);

		if (ok == false)
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		QString xml(data);
		Hash h = ::calcHash(xml);

		return h;
	}

	const std::vector<BusSignal>& Bus::busSignals() const
	{
		return m_busSignals;
	}

	std::vector<BusSignal>& Bus::busSignals()
	{
		return m_busSignals;
	}

	//
	// BusSet
	//
	bool BusSet::hasBus(QString busTypeId) const
	{
		auto it = std::find_if(m_busses.begin(), m_busses.end(),
			[&busTypeId](const VFrame30::Bus& bus)
			{
				return bus.busTypeId() == busTypeId;
			});
		return it != m_busses.end();
	}

	const VFrame30::Bus& BusSet::bus(QString busTypeId) const
	{
		auto it = std::find_if(m_busses.begin(), m_busses.end(),
			[&busTypeId](const VFrame30::Bus& bus)
			{
				return bus.busTypeId() == busTypeId;
			});

		if (it == m_busses.end())
		{
			static const VFrame30::Bus staticBus;
			return staticBus;
		}
		else
		{
			return *it;
		}
	}

	const std::vector<VFrame30::Bus>& BusSet::busses() const
	{
		return m_busses;
	}

	void BusSet::setBusses(const std::vector<VFrame30::Bus>& src)
	{
		m_busses = src;
	}
}
