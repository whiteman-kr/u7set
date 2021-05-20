#ifndef COMMON_LIB_DOMAIN
#error Don't include this file in the project! Link DbLib instead.
#endif

#include "AfbParamValue.h"
#include <optional>

namespace Afb
{
	//
	//						AfbParamValue
	//
	AfbParamValue::AfbParamValue(E::SignalType type, E::DataFormat dataFormat, quint16 size) :
		m_type(type),
		m_dataFormat(dataFormat),
		m_size(size)
	{
	}

	QString AfbParamValue::toString() const
	{
		if (m_reference.isEmpty() == false)
		{
			return m_reference;
		}
		else
		{
			return m_value.toString();
		}
	}

	QString AfbParamValue::toString(char numberFormat, int precision) const
	{
		if (m_reference.isEmpty() == false)
		{
			return m_reference;
		}

		switch (m_type)
		{
		case E::SignalType::Analog:
			{
				switch (m_dataFormat)
				{
				case E::DataFormat::Float:
					{
						float val = m_value.toFloat();
						return QString::number(val, numberFormat, precision);
					}

				case E::DataFormat::UnsignedInt:
					{
						int val = m_value.toUInt();
						return QString::number(val);
					}

				case E::DataFormat::SignedInt:
					{
						int val = m_value.toInt();
						return QString::number(val);
					}

				default:
					Q_ASSERT(false);
					return {};
				}
				break;
			}

		case E::SignalType::Discrete:
			{
				if (m_dataFormat != E::DataFormat::UnsignedInt)
				{
					Q_ASSERT(m_dataFormat == E::DataFormat::UnsignedInt);
					return {};
				}

				QString val = m_value.toUInt() ? QLatin1String("True") : QLatin1String("False");
				return val;
			}

		case E::SignalType::Bus:
			Q_ASSERT(false);
			return {};
		}

		Q_ASSERT(false);
		return {};
	}

	bool AfbParamValue::fromString(const QString& str)
	{
		// Check if string is reference
		//
		QRegExp rx("^\\$\\(([A-Za-z0-9_]+\\.)*[A-Za-z0-9_]+\\)$");	// $(AA.BB.CC)
		if (rx.exactMatch(str) == true)
		{
			m_reference = str;
			return true;
		}

		m_reference.clear();

		// Convert string to value
		//
		bool ok = false;

		switch (m_type)
		{
		case E::SignalType::Analog:
			{
				switch (m_dataFormat)
				{
				case E::DataFormat::Float:
					{
						float value = str.toFloat(&ok);
						if (ok == true)
						{
							return setValue(value);
						}
						return false;
					}

				case E::DataFormat::UnsignedInt:
					{
						uint value = str.toUInt(&ok);
						if (ok == true)
						{
							return setValue(value);
						}
						return false;
					}

				case E::DataFormat::SignedInt:
					{
						int value = str.toInt(&ok);
						if (ok == true)
						{
							return setValue(value);
						}
						return false;
					}

				default:
					Q_ASSERT(false);
					return false;
				}
				break;
			}

		case E::SignalType::Discrete:
			{
				if (m_dataFormat != E::DataFormat::UnsignedInt)
				{
					Q_ASSERT(m_dataFormat == E::DataFormat::UnsignedInt);
					return false;
				}

				bool isTrue = str.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
				bool isFalse = str.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0;

				if (isTrue == true || isFalse == true)
				{
					Q_ASSERT(isTrue * isFalse == 0);
					return setValue(isTrue);
				}

				return false;
			}

		case E::SignalType::Bus:
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(false);
		return false;
	}

	int AfbParamValue::validate(const QString& str) const
	{
		Q_ASSERT(false);
		return -1;
	}

	QVariant AfbParamValue::toVariant() const
	{
		if (bool ok = checkValue();
			ok == false)
		{
			Q_ASSERT(ok);
			return {};
		}

		return QVariant::fromValue<AfbParamValue>(*this);
	}

	bool AfbParamValue::fromVariant(const QVariant& v)
	{
		if (v.canConvert<AfbParamValue>() == false)
		{
			Q_ASSERT(v.canConvert<AfbParamValue>());
			*this = AfbParamValue{};
			return false;
		}

		*this = v.value<AfbParamValue>();
		return true;
	}

	E::SignalType AfbParamValue::type() const
	{
		return m_type;
	}

	void AfbParamValue::setType(E::SignalType type)
	{
		m_type = type;
	}

	E::DataFormat AfbParamValue::dataFormat() const
	{
		return m_dataFormat;
	}

	void AfbParamValue::setDataFormat(E::DataFormat dataFormat)
	{
		m_dataFormat = dataFormat;
	}

	bool AfbParamValue::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool AfbParamValue::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
	}

	int AfbParamValue::size() const
	{
		return m_size;
	}

	void AfbParamValue::setSize(int value)
	{
		m_size = static_cast<quint16>(value);
	}

	QVariant AfbParamValue::value() const
	{
		return m_value;
	}
	bool AfbParamValue::setValue(const QVariant& v)
	{
		m_value = v;
		return checkValue();
	}

	const QString& AfbParamValue::reference() const
	{
		return m_reference;
	}
	void AfbParamValue::setReference(const QString& value)
	{
		m_reference = value.trimmed();
	}

	bool AfbParamValue::checkValue() const
	{
		auto checkValueType = [this]<class T>(quint16 size) -> std::optional<bool>
			{
				if (m_size == size)
				{
					return {m_value.canConvert<T>()};
				}
				return {};
			};

		switch (m_type)
		{
		case E::SignalType::Analog:
			if (m_dataFormat == E::DataFormat::Float)
			{
				if (auto r = checkValueType.operator()<float>(32);
					r.has_value() == false || r.value() == false)
				{
					Q_ASSERT(false);
					return false;
				}

				return true;
			}

			if (m_dataFormat == E::DataFormat::UnsignedInt)
			{
				if (auto r = checkValueType.operator()<quint16>(16);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				if (auto r = checkValueType.operator()<quint32>(32);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				if (auto r = checkValueType.operator()<quint64>(64);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				// What size is it?
				//
				Q_ASSERT(false);
				return false;
			}

			if (m_dataFormat == E::DataFormat::SignedInt)
			{
				if (auto r = checkValueType.operator()<qint16>(16);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				if (auto r = checkValueType.operator()<qint32>(32);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				if (auto r = checkValueType.operator()<qint64>(64);
					r.has_value() == true)
				{
					if (r.value() == false)
					{
						Q_ASSERT(false);
						return false;
					}
					else
					{
						return true;
					}
				}

				// What size is it?
				//
				Q_ASSERT(false);
				return false;
			}

			// What data format is it?
			//
			Q_ASSERT(false);
			return false;

		case E::SignalType::Discrete:
			if (m_dataFormat != E::DataFormat::UnsignedInt ||
				m_value.canConvert<quint16>() == false)
			{
				Q_ASSERT(m_dataFormat == E::DataFormat::UnsignedInt);
				Q_ASSERT(m_value.canConvert<quint16>());

				return false;
			}
			return true;

		case E::SignalType::Bus:
			Q_ASSERT(false);
			return false;
		}

		return false;
	}
}
