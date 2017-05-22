#include "TrendSignal.h"

namespace TrendLib
{

	TrendSignalParam::TrendSignalParam()
	{

	}

	QString TrendSignalParam::signalId() const
	{
		return m_signalId;
	}

	void TrendSignalParam::setSignalId(const QString& value)
	{
		m_signalId = value;
	}

	QString TrendSignalParam::caption() const
	{
		return m_caption;
	}

	void TrendSignalParam::setCaption(const QString& value)
	{
		m_caption = value;
	}

	QString TrendSignalParam::equipmnetId() const
	{
		return m_equipmentId;
	}

	void TrendSignalParam::setEquipmnetId(const QString& value)
	{
		m_equipmentId = value;
	}

	bool TrendSignalParam::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool TrendSignalParam::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
	}

	E::SignalType TrendSignalParam::type() const
	{
		return m_type;
	}

	void TrendSignalParam::setType(E::SignalType value)
	{
		m_type = value;
	}

	double TrendSignalParam::lowLimit() const
	{
		return m_lowLimit;
	}

	void TrendSignalParam::setLowLimit(double value)
	{
		m_lowLimit = value;
	}

	double TrendSignalParam::highLimit() const
	{
		return m_highLimit;
	}
	void TrendSignalParam::setHighLimit(double value)
	{
		m_highLimit = value;
	}

	QString TrendSignalParam::unit() const
	{
		return m_unit;
	}

	void TrendSignalParam::setUnit(const QString& value)
	{
		m_unit = value;
	}

	QColor TrendSignalParam::color() const
	{
		return m_color;
	}

	void TrendSignalParam::setColor(const QColor& value)
	{
		m_color = value;
	}

	TrendSignalSet::TrendSignalSet()
	{
	}

	bool TrendSignalSet::addSignal(const TrendSignalParam& signal)
	{
		QMutexLocker locker(&m_paramMutex);

		auto foundIt = std::find_if(m_signalParams.begin(), m_signalParams.end(),
			[&signal](const TrendSignalParam& s)
			{
				return s.signalId() == signal.signalId();
			});

		if (foundIt != m_signalParams.end())
		{
			return false;
		}

		m_signalParams.push_back(signal);

		return true;
	}

	void TrendSignalSet::removeSignal(QString signalId)
	{
		QMutexLocker locker(&m_paramMutex);

		m_signalParams.remove_if(
			[&signalId](const TrendSignalParam& s)
			{
				return s.signalId() == signalId;
			});

		return;
	}

	std::vector<TrendSignalParam> TrendSignalSet::analogSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}

	std::vector<TrendSignalParam> TrendSignalSet::discreteSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}

	TrendSignalParam TrendSignalSet::signalParam() const
	{
		QMutexLocker locker(&m_paramMutex);

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				return s;
			}
		}

		return TrendSignalParam();
	}
}
