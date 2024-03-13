#include "../include/Behavior/MonitorBehavior.h"

namespace Behavior
{
	//
	// MonitorBehavior
	//
	const QString MonitorBehavior::nonValidityTag{"flag_nonvalid"};
	const QString MonitorBehavior::simulatedTag{"flag_simulated"};
	const QString MonitorBehavior::blockedTag{"flag_blocked"};
	const QString MonitorBehavior::mismatchTag{"flag_mismatch"};
	const QString MonitorBehavior::outOfLimitsTag{"flag_outoflimits"};
	const QString MonitorBehavior::criticalTag{"critical"};
	const QString MonitorBehavior::attentionTag{"attention"};
	const QString MonitorBehavior::generalTag{"general"};

	MonitorBehavior::MonitorBehavior()
	{
		addBaseTagToColors();
		return;
	}

	MonitorBehavior::MonitorBehavior(const MonitorBehavior& src) :
		ClientBehavior{src},
		m_tagToColors{src.m_tagToColors}
	{
	}

	MonitorBehavior::MonitorBehavior(MonitorBehavior&& src) noexcept:
		ClientBehavior{std::move(src)},
		m_tagToColors{std::move(src.m_tagToColors)}
	{
	}

	MonitorBehavior& MonitorBehavior::operator=(const MonitorBehavior& src)
	{
		if (this != &src)
		{
			ClientBehavior::operator=(src);
			m_tagToColors = src.m_tagToColors;
		}
		return *this;
	}

	MonitorBehavior& MonitorBehavior::operator=(MonitorBehavior&& src) noexcept
	{
		if (this != &src)
		{
			ClientBehavior::operator=(std::move(src));
			m_tagToColors = std::move(src.m_tagToColors);
		}
		return *this;
	}

	QStringList MonitorBehavior::tags() const
	{
		QStringList result;

		for (const auto& ttc : m_tagToColors)
		{
			result.push_back(ttc.tag);
		}

		return result;
	}

	void MonitorBehavior::setTag(int index, const QString& tag)
	{
		if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
		{
			Q_ASSERT(false);
			return;
		}

		m_tagToColors[index].tag = tag;
		return;
	}

	void MonitorBehavior::insertTagToColors(int index, const QString& tag, std::pair<QRgb, QRgb> colors)
	{
		for (auto& ttc : m_tagToColors)
		{
			if (ttc.tag == tag)
			{
				return;
			}
		}

		if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
		{
			m_tagToColors.push_back({tag, colors});
		}
		else
		{
			m_tagToColors.insert(m_tagToColors.begin() + index, {tag, colors});
		}

		return;
	}

	bool MonitorBehavior::removeTagToColors(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
		{
			Q_ASSERT(false);
			return false;
		}

		m_tagToColors.erase(m_tagToColors.begin() + index);
		return true;
	}

	bool MonitorBehavior::moveTagToColors(int index, int step)
	{
		if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
		{
			Q_ASSERT(false);
			return false;
		}

		int newIndex = index + step;

		if (newIndex < 0 || newIndex >= static_cast<int>(m_tagToColors.size()))
		{
			Q_ASSERT(false);
			return false;
		}

		std::swap(m_tagToColors[index], m_tagToColors[newIndex]);

		return true;
	}

	std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const QString& tag) const
	{
		std::optional<std::pair<QRgb, QRgb>> result;

		for (const auto& ttc : m_tagToColors)
		{
			if (ttc.tag == tag)
			{
				result = ttc.colors;
				break;
			}
		}

		return result;
	}

	void MonitorBehavior::setTagToColors(const QString& tag, std::pair<QRgb, QRgb> colors)
	{
		for (auto& ttc : m_tagToColors)
		{
			if (ttc.tag == tag)
			{
				ttc.colors = colors;
				return;
			}
		}

		m_tagToColors.push_back({tag, colors});
		return;
	}

	std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const std::set<QString>& tags) const
	{
		// Tags in m_tagToColors have priorities from the highest to the lowest.
		// So that is why regular loop is used here.
		// For future implementation:
		// for input param 'tag' if it has a lot of items and m_tagToColor quite big too,
		// is possible to make optimization, via finding interset of two tags and m_tagToColor.
		//
		std::optional<std::pair<QRgb, QRgb>> result;

		for (const auto& ttc : m_tagToColors) // Go through m_tagToColors as it is prioritized search
		{
			if (tags.find(ttc.tag) != tags.end())
			{
				result = ttc.colors;          // The first met tag has the highest priority
				break;
			}
		}

		return result;
	}

	std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const QStringList& tags) const
	{
		std::set<QString> tagSet;
		for (const auto& t : tags)
		{
			tagSet.insert(t);
		}

		return tagToColors(tagSet);
	}

	void MonitorBehavior::addBaseTagToColors()
	{
		int tag = 0;

		insertTagToColors(tag++, nonValidityTag, std::make_pair(QRgb(0xD00000), QRgb(0xD00000)));
		insertTagToColors(tag++, simulatedTag, std::make_pair(QRgb(0x0000D0), QRgb(0x0000D0)));
		insertTagToColors(tag++, blockedTag, std::make_pair(QRgb(0xD0D0D0), QRgb(0xD0D0D0)));
		insertTagToColors(tag++, mismatchTag, std::make_pair(QRgb(0xD0D000), QRgb(0xD0D000)));
		insertTagToColors(tag++, outOfLimitsTag, std::make_pair(QRgb(0xD00000), QRgb(0xD00000)));

		insertTagToColors(tag++, criticalTag, std::make_pair(QRgb(0xD00000), QRgb(0xD00000)));
		insertTagToColors(tag++, attentionTag, std::make_pair(QRgb(0xF0F000), QRgb(0xF0F000)));
		insertTagToColors(tag++, generalTag, std::make_pair(QRgb(0x0F0FF0), QRgb(0x0F0FF0)));
	}

	void MonitorBehavior::saveToXml(QXmlStreamWriter& writer)
	{
		writer.writeStartElement("SignalTagToColor");

		for (const auto& ttc : m_tagToColors)
		{
			writer.writeStartElement("Item");
			writer.writeAttribute("tag", ttc.tag);
			writer.writeAttribute("color1", QColor::fromRgb(ttc.colors.first).name());
			writer.writeAttribute("color2", QColor::fromRgb(ttc.colors.second).name());
			writer.writeEndElement();
		}

		writer.writeEndElement();
		return;
	}

	bool MonitorBehavior::loadFromXml(QXmlStreamReader& reader)
	{
		m_tagToColors.clear();

		while (reader.readNextStartElement())
		{
			// SignalTagToColor
			//
			if (reader.name() == QLatin1String("SignalTagToColor"))
			{
				continue;
			}

			// Item
			//
			if (reader.name() == QLatin1String("Item"))
			{
				QString tag;
				if (reader.attributes().hasAttribute("tag"))
				{
					tag = reader.attributes().value("tag").toString();
				}

				QColor color1;
				QColor color2;

				if (reader.attributes().hasAttribute("color"))
				{
					color1 = QColor(reader.attributes().value("color").toString());
					color2 = color1;
				}
				else
				{
					if (reader.attributes().hasAttribute("color1"))
					{
						color1 = QColor(reader.attributes().value("color1").toString());
					}

					if (reader.attributes().hasAttribute("color2"))
					{
						color2 = QColor(reader.attributes().value("color2").toString());
					}
				}

				if (tag.isEmpty() == false && color1.isValid() == true && color2.isValid() == true)
				{
					m_tagToColors.push_back({tag, std::make_pair(color1.rgb(), color2.rgb())});
				}

				reader.readNext();
				continue;
			}

			// Unknown tag
			//
			Q_ASSERT(false);
			reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
			return !reader.hasError();
		}

		// Add default tagToColors if they are not loaded
		//
		addBaseTagToColors();

		reader.readNext();
		return !reader.hasError();
	}

} // namespace ClientBehavior