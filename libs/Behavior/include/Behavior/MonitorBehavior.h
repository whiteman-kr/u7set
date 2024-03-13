#pragma once
#include "ClientBehavior.h"
#include "BehaviorColorPair.h"

#include <set>
#include <QRgb>

namespace Behavior
{
	//
	// MonitorBehavior
	//
	class MonitorBehavior : public ClientBehavior
	{
	public:
		MonitorBehavior();
		MonitorBehavior(const MonitorBehavior& src);
		MonitorBehavior(MonitorBehavior&& src) noexcept;
		virtual ~MonitorBehavior() = default;

		MonitorBehavior& operator=(const MonitorBehavior& src);
		MonitorBehavior& operator=(MonitorBehavior&& src) noexcept;

	public:
		QStringList tags() const;

		void setTag(int index, const QString& tag);

		void insertTagToColors(int index,
							   const QString& tag,
							   std::pair<QRgb, QRgb> colors); // Inserts a tag, if tag already exists, does not modify the color
		bool removeTagToColors(int index);
		bool moveTagToColors(int index, int step);

		std::optional<std::pair<QRgb, QRgb>> tagToColors(const QString& tag) const;
		void setTagToColors(const QString& tag, std::pair<QRgb, QRgb> colors); // Sets tag color, if tag does not exist, adds it

		std::optional<std::pair<QRgb, QRgb>> tagToColors(const std::set<QString>& tags) const; // Return the most priority color
		std::optional<std::pair<QRgb, QRgb>> tagToColors(const QStringList& tags) const;       // Return the most priority color

	private:
		void addBaseTagToColors();

		virtual void saveToXml(QXmlStreamWriter& writer) override;
		virtual bool loadFromXml(QXmlStreamReader& reader) override;


	public:
		static const QString criticalTag;
		static const QString attentionTag;
		static const QString generalTag;
		static const QString nonValidityTag;
		static const QString simulatedTag;
		static const QString blockedTag;
		static const QString mismatchTag;
		static const QString outOfLimitsTag;

	private:
		struct TagToColorsType
		{
			QString tag;
			std::pair<QRgb, QRgb> colors;
		};

		std::vector<TagToColorsType> m_tagToColors; // The lower position - the higher priority of the tag
	};

} // namespace Behavior
