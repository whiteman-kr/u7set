#include "Stable.h"
#include "LogicScheme.h"

namespace VFrame30
{
	LogicScheme::LogicScheme(void)
	{
		setUnit(SchemeUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	LogicScheme ::~LogicScheme (void)
	{
	}

	bool LogicScheme::SaveData(Proto::Envelope* message) const
	{
		bool result = Scheme::SaveData(message);

		if (result == false || message->has_videoframe() == false)
		{
			assert(result);
			assert(message->has_videoframe());
			return false;
		}

		// --
		//
		Proto::LogicScheme* ls = message->mutable_videoframe()->mutable_logics_scheme();

		for (const QString& strId : m_hardwareStrIds)
		{
			::Proto::wstring* hs = ls->add_hardware_strids();
			Proto::Write(hs, strId);
		}

		return true;
	}

	bool LogicScheme::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoframe() == false)
		{
			assert(message.has_videoframe());
			return false;
		}

		// --
		//
		bool result = Scheme::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoframe().has_logics_scheme() == false)
		{
			assert(message.videoframe().has_logics_scheme());
			return false;
		}

		const Proto::LogicScheme& ls = message.videoframe().logics_scheme();

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

		Scheme::Draw(pDrawParam, clipRect);
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

	void LogicScheme::setHardwareStrIds(const QString& s)
	{
		m_hardwareStrIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* LogicScheme::mutable_hardwareStrIds()
	{
		return &m_hardwareStrIds;
	}

}
