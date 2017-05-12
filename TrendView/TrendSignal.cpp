#include "TrendSignal.h"

namespace TrendLib
{

	TrendSignal::TrendSignal()
	{

	}

	QString TrendSignal::signalId() const
	{
		return m_signalId;
	}

	void TrendSignal::setSignalId(const QString& value)
	{
		m_signalId = value;
	}

	QString TrendSignal::caption() const
	{
		return m_caption;
	}

	void TrendSignal::setCaption(const QString& value)
	{
		m_caption = value;
	}

	QString TrendSignal::equipmnetId() const
	{
		return m_equipmentId;
	}

	void TrendSignal::setEquipmnetId(const QString& value)
	{
		m_equipmentId = value;
	}

	bool TrendSignal::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool TrendSignal::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
	}

	E::SignalType TrendSignal::type() const
	{
		return m_type;
	}

	void TrendSignal::setType(E::SignalType value)
	{
		m_type = value;
	}

	double TrendSignal::lowLimit() const
	{
		return m_lowLimit;
	}

	void TrendSignal::setLowLimit(double value)
	{
		m_lowLimit = value;
	}

	double TrendSignal::highLimit() const
	{
		return m_highLimit;
	}
	void TrendSignal::setHighLimit(double value)
	{
		m_highLimit = value;
	}

	QString TrendSignal::unit() const
	{
		return m_unit;
	}

	void TrendSignal::setUnit(const QString& value)
	{
		m_unit = value;
	}

	QColor TrendSignal::color() const
	{
		return m_color;
	}

	void TrendSignal::setColor(const QColor& value)
	{
		m_color = value;
	}

	TrendSignalSet::TrendSignalSet()
	{
	}

	bool TrendSignalSet::addSignal(const TrendSignal& signal)
	{
		QMutexLocker locker(&m_mutex);

		auto foundIt = std::find_if(m_signals.begin(), m_signals.end(),
			[&signal](const TrendSignal& s)
			{
				return s.signalId() == signal.signalId();
			});

		if (foundIt != m_signals.end())
		{
			return false;
		}

		m_signals.push_back(signal);

		return true;
	}

	void TrendSignalSet::removeSignal(QString signalId)
	{
		QMutexLocker locker(&m_mutex);

		m_signals.remove_if(
			[&signalId](const TrendSignal& s)
			{
				return s.signalId() == signalId;
			});

		return;
	}

	std::vector<TrendSignal> TrendSignalSet::analogSignals() const
	{
		QMutexLocker locker(&m_mutex);

		std::vector<TrendSignal> result;
		result.reserve(m_signals.size());

		for (const TrendSignal& s : m_signals)
		{
			if (s.isAnalog() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}

	std::vector<TrendSignal> TrendSignalSet::discreteSignals() const
	{
		QMutexLocker locker(&m_mutex);

		std::vector<TrendSignal> result;
		result.reserve(m_signals.size());

		for (const TrendSignal& s : m_signals)
		{
			if (s.isDiscrete() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}
}
