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

}
