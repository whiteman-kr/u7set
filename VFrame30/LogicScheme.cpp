#include "Stable.h"
#include "LogicScheme.h"

namespace VFrame30
{
	LogicScheme::LogicScheme(void)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, HardwareStrIDs, true, LogicScheme::hardwareStrIds, LogicScheme::setHardwareStrIds)

		setUnit(SchemaUnit::Inch);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(4);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Logic", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	LogicScheme ::~LogicScheme (void)
	{
	}

	bool LogicScheme::SaveData(Proto::Envelope* message) const
	{
		bool result = Schema::SaveData(message);

		if (result == false || message->has_schema() == false)
		{
			assert(result);
			assert(message->has_schema());
			return false;
		}

		// --
		//
		Proto::LogicScheme* ls = message->mutable_schema()->mutable_logics_scheme();

		for (const QString& strId : m_hardwareStrIds)
		{
			::Proto::wstring* hs = ls->add_hardware_strids();
			Proto::Write(hs, strId);
		}

		return true;
	}

	bool LogicScheme::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return false;
		}

		// --
		//
		bool result = Schema::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schema().has_logics_scheme() == false)
		{
			assert(message.schema().has_logics_scheme());
			return false;
		}

		const Proto::LogicScheme& ls = message.schema().logics_scheme();

		m_hardwareStrIds.clear();
		m_hardwareStrIds.reserve(ls.hardware_strids_size());

		for (int i = 0; i < ls.hardware_strids_size(); i++)
		{
			QString s;
			Proto::Read(ls.hardware_strids(i), &s);
			m_hardwareStrIds.push_back(s);
		}

		return true;
	}

	void LogicScheme::Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Schema::Draw(pDrawParam, clipRect);
		return;
    }

	QString LogicScheme::hardwareStrIds() const
	{
		QString result;

		for (QString s : m_hardwareStrIds)
		{
			s = s.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(s);
		}

		return result;
	}

	QStringList LogicScheme::hardwareStrIdList() const
	{
		return m_hardwareStrIds;
	}

	void LogicScheme::setHardwareStrIds(const QString& s)
	{
		m_hardwareStrIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* LogicScheme::mutable_hardwareStrIds()
	{
		return &m_hardwareStrIds;
	}

	bool LogicScheme::isMultichannelScheme() const
	{
		return m_hardwareStrIds.size() > 1;
	}

	int LogicScheme::channelCount() const
	{
		return m_hardwareStrIds.size();
	}

}
