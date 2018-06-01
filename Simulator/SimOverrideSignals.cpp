#include "SimOverrideSignals.h"
#include "SimAppSignalManager.h"
#include "SimRam.h"

namespace Sim
{

	OverrideSignalParam::OverrideSignalParam(const Signal& signalParam)
	{
		updateSignalProperties(signalParam);
		return;
	}

	void OverrideSignalParam::updateSignalProperties(const Signal& signalParam, QVariant value /*= QVariant()*/)
	{
		m_appSignalId = signalParam.appSignalID();
		m_customSignalId = signalParam.customAppSignalID();
		m_caption = signalParam.caption();
		m_equipmentId = signalParam.lmEquipmentID();

		m_signalType = signalParam.signalType();
		m_dataFormat = signalParam.analogSignalFormat();

		m_dataSizeW = signalParam.sizeW();
		m_address = signalParam.ualAddr();
		m_ramAccess = signalParam.lmRamAccess();

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
		case E::SignalType::Discrete:
			if (value.isValid() == false ||
				value.type() != m_value.type())
			{
				setDiscreteValue(0);
			}
			else
			{
				setFloatValue(m_value.value<quint16>());
			}
			break;

		case E::SignalType::Analog:
			switch (m_dataFormat)
			{
			case E::AnalogAppSignalFormat::SignedInt32:
				if (value.isValid() == false ||
					value.type() != m_value.type())
				{
					setSignedIntvalue(0);
				}
				else
				{
					setFloatValue(m_value.value<qint32>());
				}
				break;
			case E::AnalogAppSignalFormat::Float32:
				if (value.isValid() == false ||
					value.type() != m_value.type())
				{
					setFloatValue(0);
				}
				else
				{
					setFloatValue(m_value.value<float>());
				}
				break;
//			case E::AnalogAppSignalFormat::Double:???
//				break;
			default:
				assert(0);
			}
			break;

		default:
			assert(m_signalType == E::SignalType::Analog ||
				   m_signalType == E::SignalType::Discrete);
		}


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

	void OverrideSignalParam::setValue(const QVariant& value)
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

		return;
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
			m_changesCounter ++;

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
			std::optional<Signal> sp = m_appSignalManager->signalParamExt(id);

			if (sp.has_value() == false)
			{
				writeWaning(QString("Cannot add signal to override list, signal %1 not found.").arg(id));
				continue;
			}

			{
				QWriteLocker locker(&m_lock);
				m_changesCounter ++;

				auto[it, ok] = m_signals.emplace(id, *sp);
				if (ok == false)
				{
					writeWaning(QString("Signal %1 aldready added to override list.").arg(id));
					continue;
				}
				else
				{
					addedSignals << sp->appSignalID();
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
			m_changesCounter ++;

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
			m_changesCounter ++;

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

	void OverrideSignals::setValue(QString appSignalId, const QVariant& value)
	{
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			auto it = m_signals.find(appSignalId);

			if (it == m_signals.end())
			{
				writeError(tr("Can't set new value for %1, signal not found").arg(appSignalId));
				return;
			}

			OverrideSignalParam& osp = it->second;
			osp.setValue(value);
		}

		emit stateChanged(appSignalId);
		return;
	}

	void OverrideSignals::updateSignals()
	{
		std::vector<OverrideSignalParam> existingSignals =  overrideSignals();

		std::vector<OverrideSignalParam> newSignals;
		newSignals.reserve(existingSignals.size());

		for (const OverrideSignalParam& osp : existingSignals)
		{
			std::optional<Signal> sp = m_appSignalManager->signalParamExt(osp.m_appSignalId);

			if (sp.has_value() == false)
			{
				writeWaning(tr("Signal %1 removed from overriden signals.").arg(osp.m_appSignalId));
				continue;
			}

			OverrideSignalParam& updateOsp = newSignals.emplace_back(osp);
			updateOsp.updateSignalProperties(*sp, osp.m_value);
		}

		// Set updated signals
		//
		{
			QWriteLocker locker(&m_lock);
			m_changesCounter ++;

			m_signals.clear();

			for (const OverrideSignalParam& osp : newSignals)
			{
				m_signals.emplace(osp.m_appSignalId, osp);
			}
		}

		emit signalsChanged({});

		return;
	}

	std::optional<OverrideSignalParam> OverrideSignals::overrideSignal(QString appSignalId) const
	{
		std::optional<OverrideSignalParam> result;

		QReadLocker rl(&m_lock);

		auto it = m_signals.find(appSignalId);
		if (it != m_signals.end())
		{
			result = it->second;
		}

		return result;
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

	int OverrideSignals::changesCounter() const
	{
		QReadLocker rl(&m_lock);
		return m_changesCounter;
	}

	std::vector<OverrideRamRecord> OverrideSignals::ramOverrideData(QString equipmentId, const RamAreaInfo& ramAreaInfo) const
	{
		std::vector<OverrideRamRecord> result;
		E::LogicModuleRamAccess ramAccess = ramAreaInfo.access();

		// Allocate data by size of RamArea
		//
		if (ramAreaInfo.size() > 0x10000)
		{
			writeError(tr("RamArea (offset %1) in LogicModule %2 seems too big (%3)")
						.arg(ramAreaInfo.offset())
						.arg(equipmentId)
						.arg(ramAreaInfo.size()));
			return result;
		}

		result.resize(ramAreaInfo.size());

		// --
		//
		QReadLocker locker(&m_lock);

		for (auto[appSignalId, osp] : m_signals)
		{
			if (osp.m_ramAccess != ramAccess ||			// Signal is not in this RAM Area
				osp.m_equipmentId != equipmentId)		// Signal is not in this LM
			{
				continue;
			}

			int dataSizeW = osp.m_dataSizeW;
			int offsetW = osp.m_address.offset();

			if (offsetW < static_cast<int>(ramAreaInfo.offset()) ||
				offsetW >= static_cast<int>(ramAreaInfo.offset() + ramAreaInfo.size()))
			{
				// Signal is not in this RamArea
				// dataSizeW is not taken into checks, as we suppose that signal can be in only area
				//
				continue;
			}

			offsetW -= ramAreaInfo.offset();	// Make it 0-based

			if (offsetW < 0 || offsetW + dataSizeW > result.size())
			{
				assert(false);
				return result;
			}

			for (int i = 0; i < dataSizeW; i++)
			{
				result[offsetW].overlapRecord(osp.m_ramOverrides[i]);
				offsetW++;
			}
		}

		return result;
	}

}
