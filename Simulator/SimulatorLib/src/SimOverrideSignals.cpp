#include "SimOverrideSignalsImpl.h"

namespace Sim
{
	//
	// OverrideSignalParam
	//
	const QString OverrideSignalParam::s_defaultScriptValue = R"+++((function(lastOverrideValue, workcycle)
{
	// lastOverrideValue - The last value returned from this function
	// workcycle - Workcycle counter

	var result = 0;

	return result;	// Returns value for signal overriding
}))+++";

	OverrideSignalParam::OverrideSignalParam(const OverrideSignalParam& src)
	{
		*this = src;
	}

	OverrideSignalParam::OverrideSignalParam(const AppSignal& signalParam)
	{
		updateSignalProperties(signalParam);
		return;
	}

	OverrideSignalParam::~OverrideSignalParam() = default;

	OverrideSignalParam& OverrideSignalParam::operator=(const OverrideSignalParam& src)
	{
		this->m_enabled = src.m_enabled;
		this->m_index = src.m_index;

		this->m_appSignalId = src.m_appSignalId;
		this->m_customSignalId = src.m_customSignalId;
		this->m_caption = src.m_caption;
		this->m_lmEquipmentId = src.m_lmEquipmentId;

		this->m_signalType = src.m_signalType;
		this->m_dataFormat = src.m_dataFormat;
		this->m_byteOrder = src.m_byteOrder;

		this->m_dataSizeW = src.m_dataSizeW;
		this->m_address = src.m_address;
		this->m_ramAccess = src.m_ramAccess;

		this->m_ramOverrides = src.m_ramOverrides;

		this->m_method = src.m_method;
		this->m_value = src.m_value;
		this->m_script = src.m_script;

		return *this;
	}

	void OverrideSignalParam::updateSignalProperties(const AppSignal& signalParam, QVariant value /*= QVariant()*/)
	{
		m_appSignalId = signalParam.appSignalID();
		m_customSignalId = signalParam.customAppSignalID();
		m_caption = signalParam.caption();
		m_lmEquipmentId = signalParam.lmEquipmentID();

		m_signalType = signalParam.signalType();
		m_dataFormat = signalParam.analogSignalFormat();
		m_byteOrder = signalParam.byteOrder();

		m_dataSizeW = signalParam.sizeW();
		m_address = signalParam.actualAddr();
		m_ramAccess = signalParam.lmRamAccess();

		// Checks
		//
		if (m_dataSizeW > static_cast<int>(m_ramOverrides.size()))
		{
			Q_ASSERT(m_dataSizeW <= static_cast<int>(m_ramOverrides.size()));
			return;
		}

		// Create mask/data records
		//
		switch (m_signalType)
		{
		case E::SignalType::Discrete:
			if (value.isValid() == false || value.typeId() != m_value.typeId())
			{
				setDiscreteValue(0);
			}
			else
			{
				setDiscreteValue(m_value.value<quint16>());
			}
			break;

		case E::SignalType::Analog:
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				if (value.isValid() == false || value.typeId() != m_value.typeId())
				{
					setSignedIntvalue(0);
				}
				else
				{
					setSignedIntvalue(m_value.value<qint32>());
				}
				break;
			case E::AnalogAppSignalFormat::Float32:
				if (value.isValid() == false || value.typeId() != m_value.typeId())
				{
					setFloatValue(0);
				}
				else
				{
					setFloatValue(m_value.value<float>());
				}
				break;
			default:
				assert(0);
			}
			break;

		default:
			assert(m_signalType == E::SignalType::Analog || m_signalType == E::SignalType::Discrete);
		}

		return;
	}


	QString OverrideSignalParam::valueString(int base /*= 10*/,
											 E::AnalogFormat analogFormat /*= E::AnalogFormat::g_9_or_9e*/,
											 int precision /*= -14*/) const
	{
		QString result;

		if (m_signalType != E::SignalType::Discrete && m_signalType != E::SignalType::Analog)
		{
			assert(m_signalType == E::SignalType::Discrete || m_signalType == E::SignalType::Analog);

			return result;
		}

		if (m_signalType == E::SignalType::Discrete)
		{
			assert(m_value.canConvert<quint16>() == true);

			quint16 val = m_value.value<quint16>();

			result = QString{"%1"}.arg(val);
		}

		if (m_signalType == E::SignalType::Analog)
		{
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				{
					qint32 val = m_value.value<qint32>();

					if (base == 10)
					{
						result = QString{"%1"}.arg(val, 0, base);
					}

					if (base == 16)
					{
						result = QString{"%1h"}.arg(val, 0, base);
					}
				}
				break;
			case E::AnalogAppSignalFormat::Float32:
				{
					float val = m_value.value<float>();


					if (precision == -1)
					{
						result = QString{"%1"}.arg(val, 0, (char)(analogFormat));
					}
					else
					{
						result = QString{"%1"}.arg(val, 0, (char)(analogFormat), precision);
					}

					QLocale c;
					result.replace('.', c.decimalPoint());
				}
				break;
			default:
				assert(false);
			}
		}

		if (m_method == OverrideSignalMethod::Script)
		{
			if (m_scriptError.isEmpty() == false)
			{
				result = QLatin1String("JS: ") + m_scriptError;
			}
			else
			{
				result.prepend(QLatin1String("JS: "));
			}
		}


		return result;
	}

	void OverrideSignalParam::setValue(const QVariant& value, OverrideSignalMethod method, bool changeCurrentMethod)
	{
		if (changeCurrentMethod == true)
		{
			m_method = method;
		}

		switch (method)
		{
		case OverrideSignalMethod::Value:
			{
				switch (m_signalType)
				{
				case E::SignalType::Discrete:
					{
						if (value.canConvert<quint16>() == false)
						{
							assert(value.canConvert<quint16>());
							break;
						}

						quint16 discrValue = value.value<quint16>();
						setDiscreteValue(discrValue);
					}
					break;

				case E::SignalType::Analog:
					{
						switch (m_dataFormat)
						{
						case E::AnalogAppSignalFormat::SignedInt32:
							{
								if (value.canConvert<qint32>() == false)
								{
									assert(value.canConvert<qint32>());
									break;
								}

								qint32 sintValue = value.value<qint32>();
								setSignedIntvalue(sintValue);
							}
							break;

						case E::AnalogAppSignalFormat::Float32:
							{
								if (value.canConvert<float>() == false)
								{
									assert(value.canConvert<float>());
									break;
								}

								float floatValue = value.value<float>();
								setFloatValue(floatValue);
							}
							break;
						default:
							assert(false);
						}
					}
					break;

				default:
					assert(false);
					break;
				}
			}
			break;

		case OverrideSignalMethod::Script:
			{
				m_scriptError.clear();
				m_scriptValueRequiresReset = true;

				if (value.typeId() == QMetaType::QString)
				{
					m_script = value.toString();

					switch (m_signalType)
					{
					case E::SignalType::Discrete:
						setDiscreteValue(0);
						break;

					case E::SignalType::Analog:
						switch (m_dataFormat)
						{
						case E::AnalogAppSignalFormat::SignedInt32:
							setSignedIntvalue(0);
							break;
						case E::AnalogAppSignalFormat::Float32:
							setFloatValue(0);
							break;
						default:
							assert(false);
						}
						break;
					default:
						assert(false);
						break;
					}
				}
				else
				{
					assert(value.typeId() == QMetaType::QString);
					m_value = QString("");
				}
			}
			break;

		default:
			assert(false);
		}

		return;
	}

	void OverrideSignalParam::setDiscreteValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		value = value ? 0x0001 : 0x0000;

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0x0001 << m_address.bit());
		m_ramOverrides[0].data = qToBigEndian<quint16>(value << m_address.bit());

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setWordValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = qToBigEndian<quint16>(value);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setSignedIntvalue(qint32 value)
	{
		assert(m_dataSizeW == 2);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			qint32 beFloat = qToBigEndian(value);

#ifdef __cpp_lib_bit_cast
			quint32 asDword = std::bit_cast<quint32>(beFloat);
#else
			quint32 asDword;
			std::memcpy(&asDword, &beFloat, sizeof(beFloat));
#endif

			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = static_cast<quint16>(asDword & 0xFFFF);

			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].data = static_cast<quint16>(asDword >> 16);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setFloatValue(float value)
	{
		assert(m_dataSizeW == 2);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			float beFloat = qToBigEndian(value);
#ifdef __cpp_lib_bit_cast
			quint32 asDword = std::bit_cast<quint32>(beFloat);
#else
			quint32 asDword;
			std::memcpy(&asDword, &beFloat, sizeof(beFloat));
#endif
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[0].data = static_cast<quint16>(asDword & 0xFFFF);

			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].data = static_cast<quint16>(asDword >> 16);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setDoubleValue(double value)
	{
		assert(m_dataSizeW == 4);

		if (m_byteOrder == E::ByteOrder::BigEndian)
		{
			double beFloat = qToBigEndian(value);
#ifdef __cpp_lib_bit_cast
			quint64 asDwword = std::bit_cast<quint64>(beFloat);
#else
			quint64 asDwword;
			std::memcpy(&asDwword, &beFloat, sizeof(beFloat));
#endif
			m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[2].mask = qToBigEndian<quint16>(0xFFFF);
			m_ramOverrides[3].mask = qToBigEndian<quint16>(0xFFFF);

			m_ramOverrides[0].data = static_cast<quint16>(asDwword & 0xFFFF);
			m_ramOverrides[1].data = static_cast<quint16>((asDwword >> 16) & 0xFFFF);
			m_ramOverrides[2].data = static_cast<quint16>((asDwword >> 32) & 0xFFFF);
			m_ramOverrides[3].data = static_cast<quint16>(asDwword >> 48);
		}
		else
		{
			// To do
			//
			assert(false);
		}

		m_value = QVariant::fromValue(value);

		return;
	}

	bool OverrideSignalParam::sameType(const OverrideSignalParam& another) const
	{
		return (signalType() == E::SignalType::Discrete && another.signalType() == E::SignalType::Discrete) ||
			   (signalType() == E::SignalType::Analog && another.signalType() == E::SignalType::Analog &&
				dataFormat() == another.dataFormat());
	}

	bool OverrideSignalParam::enabled() const
	{
		return m_enabled;
	}

	void OverrideSignalParam::setEnabled(bool en)
	{
		m_enabled = en;
	}

	int OverrideSignalParam::index() const
	{
		return m_index;
	}

	void OverrideSignalParam::setIndex(int value)
	{
		m_index = value;
	}

	const QString& OverrideSignalParam::appSignalId() const
	{
		return m_appSignalId;
	}

	const QString& OverrideSignalParam::customSignalId() const
	{
		return m_customSignalId;
	}

	const QString& OverrideSignalParam::caption() const
	{
		return m_caption;
	}

	const QString& OverrideSignalParam::lmEquipmentId() const
	{
		return m_lmEquipmentId;
	}

	E::SignalType OverrideSignalParam::signalType() const
	{
		return m_signalType;
	}

	E::AnalogAppSignalFormat OverrideSignalParam::dataFormat() const
	{
		return m_dataFormat;
	}

	E::ByteOrder OverrideSignalParam::byteOrder() const
	{
		return m_byteOrder;
	}

	int OverrideSignalParam::dataSizeW() const
	{
		return m_dataSizeW;
	}

	const Address16& OverrideSignalParam::address() const
	{
		return m_address;
	}

	E::LogicModuleRamAccess OverrideSignalParam::ramAccess() const
	{
		return m_ramAccess;
	}

	const OverrideRamRecord& OverrideSignalParam::ramOverrides(size_t index) const
	{
		assert(index <= m_ramOverrides.size());
		return m_ramOverrides[index];
	}

	OverrideSignalMethod OverrideSignalParam::method() const
	{
		return m_method;
	}

	const QVariant& OverrideSignalParam::value() const
	{
		return m_value;
	}

	const QString& OverrideSignalParam::script() const
	{
		return m_script;
	}

	const QString& OverrideSignalParam::scriptError() const
	{
		return m_scriptError;
	}

	void OverrideSignalParam::setScriptError(const QString& value)
	{
		m_scriptError = value;
	}


	//
	// OverrideSignalsImpl
	//
	OverrideSignals::OverrideSignals(OverrideSignalsImpl& m_impl, QObject* parent) :
		QObject{parent},
		m_impl{m_impl}
	{
		connect(&m_impl, &OverrideSignalsImpl::signalsChanged, this, &OverrideSignals::signalsChanged);
		connect(&m_impl, &OverrideSignalsImpl::stateChanged, this, &OverrideSignals::stateChanged);
	}

	void OverrideSignals::clear()
	{
		return m_impl.clear();
	}

	int OverrideSignals::addSignals(const QStringList& appSignalIds)
	{
		return m_impl.addSignals(appSignalIds);
	}

	bool OverrideSignals::addSignal(QString appSignalId,
									bool enabled,
									int index,
									OverrideSignalMethod method,
									QVariant value,
									QString script)
	{
		return m_impl.addSignal(appSignalId, enabled, index, method, value, script);
	}

	void OverrideSignals::removeSignal(const QString& appSignalId)
	{
		return m_impl.removeSignal(appSignalId);
	}

	void OverrideSignals::removeSignals(const QStringList& appSignalIds)
	{
		return m_impl.removeSignals(appSignalIds);
	}

	bool OverrideSignals::containsSignal(const QString& appSignalId) const
	{
		return m_impl.containsSignal(appSignalId);
	}

	void OverrideSignals::setEnable(QString appSignalId, bool enable)
	{
		return m_impl.setEnable(appSignalId, enable);
	}

	void OverrideSignals::setValue(QString appSignalId, OverrideSignalMethod method, const QVariant& value)
	{
		return m_impl.setValue(appSignalId, method, value);
	}

	void OverrideSignals::setValues(const std::vector<OverrideSetValueData>& overrideData)
	{
		return m_impl.setValues(overrideData);
	}

	bool OverrideSignals::saveWorkspace(QString fileName) const
	{
		return m_impl.saveWorkspace(fileName);
	}

	bool OverrideSignals::loadWorkspace(QString fileName)
	{
		return m_impl.loadWorkspace(fileName);
	}

	std::optional<OverrideSignalParam> OverrideSignals::overrideSignal(QString appSignalId) const
	{
		return m_impl.overrideSignal(appSignalId);
	}

	std::vector<OverrideSignalParam> OverrideSignals::overrideSignals() const
	{
		return m_impl.overrideSignals();
	}

	QStringList OverrideSignals::overrideSignalIds() const
	{
		return m_impl.overrideSignalIds();
	}
} // namespace Sim