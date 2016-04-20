#include "Stable.h"
#include "LogicSchema.h"

namespace VFrame30
{
	LogicSchema::LogicSchema(void)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, HardwareStrIDs, true, LogicSchema::hardwareStrIds, LogicSchema::setHardwareStrIds)

		setUnit(SchemaUnit::Inch);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(4);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemaLayer>("Logic", true));
		Layers.push_back(std::make_shared<SchemaLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemaLayer>("Notes", false));

		return;
	}

	LogicSchema ::~LogicSchema (void)
	{
	}

	bool LogicSchema::SaveData(Proto::Envelope* message) const
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
		Proto::LogicSchema* ls = message->mutable_schema()->mutable_logic_schema();

		for (const QString& strId : m_hardwareStrIds)
		{
			::Proto::wstring* hs = ls->add_equipmentids();
			Proto::Write(hs, strId);
		}

		ls->set_counter(m_counter);

		return true;
	}

	bool LogicSchema::LoadData(const Proto::Envelope& message)
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
		if (message.schema().has_logic_schema() == false)
		{
			assert(message.schema().has_logic_schema());
			return false;
		}

		const Proto::LogicSchema& ls = message.schema().logic_schema();

		m_hardwareStrIds.clear();
		m_hardwareStrIds.reserve(ls.equipmentids_size());

		for (int i = 0; i < ls.equipmentids_size(); i++)
		{
			QString s;
			Proto::Read(ls.equipmentids(i), &s);
			m_hardwareStrIds.push_back(s);
		}

		m_counter = ls.counter();

		return true;
	}

	void LogicSchema::Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Schema::Draw(pDrawParam, clipRect);
		return;
    }

	QString LogicSchema::hardwareStrIds() const
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

	QStringList LogicSchema::hardwareStrIdList() const
	{
		return m_hardwareStrIds;
	}

	void LogicSchema::setHardwareStrIds(const QString& s)
	{
		m_hardwareStrIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* LogicSchema::mutable_hardwareStrIds()
	{
		return &m_hardwareStrIds;
	}

	bool LogicSchema::isMultichannelSchema() const
	{
		return m_hardwareStrIds.size() > 1;
	}

	int LogicSchema::channelCount() const
	{
		return m_hardwareStrIds.size();
	}

	int LogicSchema::nextCounterValue()
	{
		return ++m_counter;
	}

}
