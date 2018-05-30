#include "SimOverrideSignals.h"

namespace Sim
{

	OverrideSignalParam::OverrideSignalParam(const Signal& signalParam)
	{
		m_appSignalId = signalParam.appSignalID();
		m_customSignalId = signalParam.customAppSignalID();
		m_caption = signalParam.caption();
		m_equipmentId = signalParam.lmEquipmentID();

		m_signalType = signalParam.signalType();
		m_dataFormat = signalParam.analogSignalFormat();

		m_dataSizeW = signalParam.sizeW();
		m_address = signalParam.ualAddr();

		// Checks
		//
		if (m_dataSizeW > m_ramOverrides.size())
		{
			assert(m_dataSizeW <= m_ramOverrides.size());
			return;
		}

		// Create mask/data records
		//
		switch (m_signalType)
		{
		case E::SignalType::Analog:
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				setSignedIntvalue(0);
				break;
			case E::AnalogAppSignalFormat::Float32:
				setFloatValue(0);
				break;
//			case E::AnalogAppSignalFormat::Double:???
//				break;
			default:
				assert(0);
			}
			break;

		case E::SignalType::Discrete:
			setDiscreteValue(0);
			break;

		default:
			assert(m_signalType == E::SignalType::Analog ||
				   m_signalType == E::SignalType::Discrete);
		}

		return;
	}

	QString OverrideSignalParam::valueString(int base /*= 10*/,
											 E::AnalogFormat analogFormat /*= E::AnalogFormat::g_9_or_9e*/,
											 int precision /*= -14*/) const
	{
		QString result;

		if (m_signalType != E::SignalType::Discrete &&
			m_signalType != E::SignalType::Analog)
		{
			assert(m_signalType == E::SignalType::Discrete ||
				   m_signalType == E::SignalType::Analog);

			return result;
		}

		if (m_signalType == E::SignalType::Discrete)
		{
			assert(m_value.canConvert<quint16>() == true);

			quint16 val = m_value.value<quint16>();

			result = QString{"%1"}.arg(val);
			return result;
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
				}
				break;
			default:
				assert(false);
			}

			return result;
		}

		assert(false);
		return result;
	}

	void OverrideSignalParam::setDiscreteValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0x0001 << m_address.bit());
		m_ramOverrides[0].data = qToBigEndian<quint16>((value & 0x0001) << m_address.bit());

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setWordValue(quint16 value)
	{
		assert(m_dataSizeW == 1);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[0].data = qToBigEndian<quint16>(value);

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setSignedIntvalue(qint32 value)
	{
		assert(m_dataSizeW == 2);

		qint32 converted = qToBigEndian<qint32>(value);
		quint16* ptr = reinterpret_cast<quint16*>(&converted);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[0].data = *ptr;

		m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[1].data = *(ptr + 1);

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setFloatValue(float value)
	{
		assert(m_dataSizeW == 2);

		union Converter
		{
			quint32 asDword;
			float asFloat;
		};

		Converter c;
		c.asFloat = value;
		c.asDword = qToBigEndian(c.asDword);

		quint16* ptr = reinterpret_cast<quint16*>(&c.asDword);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[0].data = *ptr;

		m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[1].data = *(ptr + 1);

		m_value = QVariant::fromValue(value);

		return;
	}

	void OverrideSignalParam::setDoubleValue(double value)
	{
		assert(m_dataSizeW == 4);

		union Converter
		{
			quint64 asDdword;
			double asDouble;
		};

		Converter c;
		c.asDouble = value;

		quint16* ptr = reinterpret_cast<quint16*>(c.asDdword);

		m_ramOverrides[0].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[0].data = *ptr;

		m_ramOverrides[1].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[1].data = *(ptr + 1);

		m_ramOverrides[2].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[2].data = *(ptr + 2);

		m_ramOverrides[3].mask = qToBigEndian<quint16>(0xFFFF);
		m_ramOverrides[3].data = *(ptr + 3);

		m_value = QVariant::fromValue(value);

		return;
	}


	OverrideSignals::OverrideSignals(Sim::AppSignalManager* appSignalManager, QObject* parent /*= nullptr*/) :
		QObject(parent),
		Output("OverrideSignals"),
		m_appSignalManager(appSignalManager)
	{
		assert(appSignalManager);
		return;
	}

	OverrideSignals::~OverrideSignals()
	{
	}

	void OverrideSignals::clear()
	{
		{
			QWriteLocker locker(&m_lock);
			m_signals.clear();
		}

		emit signalsChanged({});
		return;
	}

	int OverrideSignals::addSignals(const QStringList& appSignalIds)
	{
		if (m_appSignalManager == nullptr)
		{
			assert(m_appSignalManager);
			return 0;
		}

		QStringList addedSignals;

		for (const QString& id : appSignalIds)
		{
			bool found = false;
			Signal sp = m_appSignalManager->signalParamExt(id, &found);

			if (found == false)
			{
				writeWaning(QString("Cannot add signal to override list, signal %1 not found.").arg(id));
				continue;
			}

			{
				QWriteLocker locker(&m_lock);

				auto[it, ok] = m_signals.emplace(id, sp);
				if (ok == false)
				{
					writeWaning(QString("Signal %1 aldready added to override list.").arg(id));
					continue;
				}
				else
				{
					addedSignals << sp.appSignalID();
				}
			}
		}

		if (addedSignals.isEmpty() == false)
		{
			emit signalsChanged(addedSignals);
		}

		return addedSignals.size();
	}

	void OverrideSignals::removeSignal(QString appSignalId)
	{
		{
			QWriteLocker locker(&m_lock);
			m_signals.erase(appSignalId);
		}

		emit signalsChanged({});
		return;
	}

	void OverrideSignals::setEnable(QString appSignalId, bool enable)
	{
		bool changed = false;

		{
			QWriteLocker locker(&m_lock);

			if (auto it = m_signals.find(appSignalId);
				it != m_signals.end() && it->second.m_enabled != enable)
			{
				it->second.m_enabled = enable;
				changed = true;
			}
		}

		if (changed == true)
		{
			emit stateChanged(appSignalId);
		}

		return;
	}

	std::vector<OverrideSignalParam> OverrideSignals::overrideSignals() const
	{
		std::vector<OverrideSignalParam> result;

		QReadLocker rl(&m_lock);

		result.reserve(m_signals.size());

		for (auto[appSignalId, ovSignalParam] :  m_signals)
		{
			result.push_back(ovSignalParam);
		}

		return result;
	}

}
