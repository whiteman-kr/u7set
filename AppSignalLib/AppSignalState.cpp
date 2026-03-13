#include "AppSignalState.h"

AppSignalState::AppSignalState(const Proto::AppSignalState& protoState)
{
	try
	{
		load(protoState);
	}
	catch (...)
	{
	}
}

AppSignalState::AppSignalState(Hash hash, Times times, double value, AppSignalStateFlags flags) :
	m_hash{hash},
	m_time{times},
	m_value{value},
	m_flags{flags}
{
}

Hash AppSignalState::hash() const
{
	return m_hash;
}

const Times& AppSignalState::time() const
{
	return m_time;
}

const TimeStamp& AppSignalState::time(E::TimeType timeType) const
{
	switch (timeType)
	{
	case E::TimeType::Plant:
		return m_time.plant;
	case E::TimeType::System:
		return m_time.system;
	case E::TimeType::Local:
		return m_time.local;
	default:
		{
			static const TimeStamp dummy;
			return dummy;
		}
	}
}

double AppSignalState::value() const noexcept
{
	return m_value;
}

bool AppSignalState::isValid() const noexcept
{
	return m_flags.valid;
}

bool AppSignalState::isStateAvailable() const
{
	return m_flags.stateAvailable;
}

bool AppSignalState::isSimulated() const
{
	return m_flags.simulated;
}

bool AppSignalState::isBlocked() const
{
	return m_flags.blocked;
}

bool AppSignalState::isMismatch() const
{
	return m_flags.mismatch;
}

bool AppSignalState::isAboveHighLimit() const
{
	return m_flags.aboveHighLimit;
}

bool AppSignalState::isBelowLowLimit() const
{
	return m_flags.belowLowLimit;
}

bool AppSignalState::isOutOfLimits() const
{
	return isAboveHighLimit() || isBelowLowLimit();
}

bool AppSignalState::isTuningDefault() const
{
	return m_flags.tuningDefault;
}

void AppSignalState::clear()
{
	m_hash = 0;
	m_time.system.timeStamp = 0;
	m_time.local.timeStamp = 0;
	m_time.plant.timeStamp = 0;
	m_value = 0;
	m_flags.all = 0;
}

Proto::AppSignalState AppSignalState::save() const
{
	Proto::AppSignalState protoState;
	save(&protoState);
	return protoState;
}

void AppSignalState::save(Proto::AppSignalState* protoState) const
{
	if (protoState == nullptr)
	{
		assert(false);
		return;
	}

	assert(m_hash != 0);

	protoState->set_hash(m_hash);
	protoState->set_value(m_value);
	protoState->set_flags(m_flags.all);

	protoState->set_systemtime(m_time.system.timeStamp);
	protoState->set_localtime(m_time.local.timeStamp);
	protoState->set_planttime(m_time.plant.timeStamp);

	return;
}

Hash AppSignalState::load(const Proto::AppSignalState& protoState)
{
	m_hash = protoState.hash();
	assert(m_hash != 0);

	m_value = protoState.value();
	m_flags.all = protoState.flags();

	m_time.system.timeStamp = protoState.systemtime();
	m_time.local.timeStamp = protoState.localtime();
	m_time.plant.timeStamp = protoState.planttime();

	return m_hash;
}

bool AppSignalState::hasSameValue(const AppSignalState& b) const
{
	return m_flags.all == b.m_flags.all && m_value == b.m_value && m_hash == b.m_hash;
}

QString AppSignalState::toString(double value,
								 E::ValueViewType viewType,
								 E::AnalogFormat analogFormat,
								 E::AnalogAppSignalFormat analogAppSignalFormat,
								 int precision)
{
	QString result;
	result.reserve(64);

	int p = 4;

	float floatValue = static_cast<float>(value);

#ifdef __cpp_lib_bit_cast
	quint32 floatValueBits = std::bit_cast<quint32>(floatValue);
	quint64 doubleValueBits = std::bit_cast<quint64>(value);
#else
	quint32 floatValueBits;
	quint64 doubleValueBits;

	std::memcpy(&floatValueBits, &floatValue, sizeof(floatValue));
	std::memcpy(&doubleValueBits, &value, sizeof(value));
#endif

	switch (viewType)
	{
	case E::ValueViewType::Dec:
		result = QString::number(value, static_cast<char>(analogFormat), precision);
		break;

	case E::ValueViewType::Hex:
		if (analogAppSignalFormat == E::AnalogAppSignalFormat::SignedInt32)
		{
			result = QString::number((long)value, 16).leftJustified(16, '0') + QStringLiteral("h");
		}
		else
		{
			Q_ASSERT(analogAppSignalFormat == E::AnalogAppSignalFormat::Float32);

			result = QStringLiteral("FP IEEE 754: ") + QString::number(floatValueBits, 16).leftJustified(8, '0') + QStringLiteral("h");

			if (std::isnan(floatValue) == true)
			{
				result += QStringLiteral(" (nan)");
			}
			if (std::isinf(floatValue) == true)
			{
				result += floatValue == -std::numeric_limits<float>::infinity() ? QStringLiteral(" (-inf)") : QStringLiteral(" (inf)");
			}
		}
		break;

	case E::ValueViewType::Exp:
		result = QString::number(value, 'e', precision);
		break;

	case E::ValueViewType::Bin32:
		if (analogAppSignalFormat == E::AnalogAppSignalFormat::SignedInt32)
		{
			result = QString::number((quint32)floatValue, 2);
			result = result.rightJustified(32, '0');
			for (int q = 0; q < 7; q++, p += 5)
			{
				result.insert(p, ' ');
			}
		}
		else
		{
			Q_ASSERT(analogAppSignalFormat == E::AnalogAppSignalFormat::Float32);

			// Print IEEE 564 format for Float number

			result = QString::number(floatValueBits, 2).rightJustified(32, '0');
			result.insert(1, QStringLiteral(" E:"));
			result.insert(result.length() - 23, QStringLiteral(" M:"));
			result = QStringLiteral("FP IEEE 754: S:") + result;

			if (std::isnan(floatValue) == true)
			{
				result += QStringLiteral(" (nan)");
			}
			if (std::isinf(floatValue) == true)
			{
				result += floatValue == -std::numeric_limits<float>::infinity() ? QStringLiteral(" (-inf)") : QStringLiteral(" (inf)");
			}
		}
		break;

	case E::ValueViewType::Bin64:
		if (analogAppSignalFormat == E::AnalogAppSignalFormat::SignedInt32)
		{
			result = QString::number((quint64)doubleValueBits, 2);
			result = result.rightJustified(64, '0');
			for (int q = 0; q < 15; q++, p += 5)
			{
				result.insert(p, ' ');
			}
			result.insert(40, QChar::LineFeed);
		}
		else
		{
			Q_ASSERT(analogAppSignalFormat == E::AnalogAppSignalFormat::Float32);

			// Print IEEE 564 format for Double number

			result = QString::number(doubleValueBits, 2).rightJustified(64, '0');
			result.insert(1, QStringLiteral(" E:"));
			result.insert(result.length() - 52, QStringLiteral(" M:"));
			result = QStringLiteral("DBL IEEE 754: S:") + result;

			if (std::isnan(value) == true)
			{
				result += QStringLiteral(" (nan)");
			}
			if (std::isinf(value) == true)
			{
				result += value == -std::numeric_limits<float>::infinity() ? QStringLiteral(" (-inf)") : QStringLiteral(" (inf)");
			}
		}
		break;

	default:
		assert(false);
	}

	return result;
}
