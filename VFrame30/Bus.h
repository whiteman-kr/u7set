#ifndef BUS_H
#define BUS_H

#include "../lib/PropertyObject.h"
#include "../lib/Types.h"

class QDomElement;

namespace VFrame30
{
	//
	// BusSignal
	//
	class VFRAME30LIBSHARED_EXPORT BusSignal : public PropertyObject
	{
	public:
		BusSignal();
		BusSignal(const BusSignal& src);
		BusSignal(E::SignalType type);
		BusSignal& operator= (const BusSignal& src)
		{
			if (this != &src)
			{
				this->m_name = src.m_name;
				this->m_type = src.m_type;
				this->m_analogDataFormat = src.m_analogDataFormat;
				this->m_busTypeId = src.m_busTypeId;
			}
			return *this;
		}

		bool load(const QDomElement& domElement, QString* errorMessage);
		bool save(QDomElement* domElement) const;

	public:
		QString name() const;
		void setName(const QString& value);

		E::SignalType type() const;
		void setType(E::SignalType value);

		E::AnalogAppSignalFormat analogFormat() const;
		void setAnalogFormat(E::AnalogAppSignalFormat value);

		QString busTypeId() const;
		void setBusTypeId(const QString& value);

	private:
		QString m_name = "name";
		E::SignalType m_type = E::SignalType::Discrete;
		E::AnalogAppSignalFormat m_analogDataFormat = E::AnalogAppSignalFormat::Float32;	// Use only if m_type is Analog
		QString m_busTypeId = "BUSTYPEID";													// Use only if m_type is Bus
	};

	//
	// Bus
	//
	class VFRAME30LIBSHARED_EXPORT Bus
	{
	public:
		Bus();

		bool load(const QByteArray& data, QString* errorMessage);
		bool save(QByteArray* data) const;

	public:
		QString busTypeId() const;
		void setBusTypeId(const QString& value);

		int version() const;
		void incrementVersion();

		const std::vector<BusSignal>& busSignals() const;

	private:
		QString m_busTypeId = "BUSTYPEID";
		int m_version = -1;
		std::vector<BusSignal> m_busSignals;
	};

}

#endif // BUS_H
