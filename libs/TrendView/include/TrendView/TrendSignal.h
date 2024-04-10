#ifndef TRENDSIGNAL_H
#define TRENDSIGNAL_H

#include <set>
#include <map>

#include "TrendArchiveServer.h"

using TrendColor = quint32;		// This is QRgb, the problem is this header is used buy other libs without GUI (simulator)

namespace Proto
{
	class TrendSignalParam;
}

namespace TrendLib
{

	struct TrendViewLimits
	{
		double highLimit = 1.0;
		double lowLimit = 0.0;
	};


	class TrendSignalParam
	{
	public:
		TrendSignalParam();
		TrendSignalParam(const AppSignalParam& appSignal,
						 const TrendLib::ArchiveServer& archiveServer);

	public:
		bool save(Proto::TrendSignalParam* message) const;
		bool load(const Proto::TrendSignalParam& message);

		// Methods
		//
	public:
		//[[nodiscard]] AppSignalParam toAppSignalParam() const;

		// Properties
		//
	public:
		[[nodiscard]] TrendSignalPlusServerId signalPlusServerId() const;	// This id is composed form AppSignalID and archiveServerId

		[[nodiscard]] const QString& signalId() const;
		void setSignalId(const QString& value);

		[[nodiscard]] const QString& appSignalId() const;
		void setAppSignalId(const QString& value);

		[[nodiscard]] Hash appSignalHash() const;

		[[nodiscard]] const QString& caption() const;
		void setCaption(const QString& value);

		[[nodiscard]] const QString& equipmnetId() const;
		void setEquipmnetId(const QString& value);

		[[nodiscard]] const QString& archiveServerId() const;
		void setArchiveServerId(const QString& value);

		[[nodiscard]] const QString& archiveServerShortId() const;
		void setArchiveServerShortId(const QString& value);

		[[nodiscard]] const ArchiveServer& archiveServer() const;
		void setArchiveServer(const ArchiveServer& value);

		[[nodiscard]] bool isAnalog() const;
		[[nodiscard]] bool isDiscrete() const;
		[[nodiscard]] E::SignalType type() const;
		void setType(E::SignalType value);

		[[nodiscard]] const QString& unit() const;
		void setUnit(const QString& value);

		[[nodiscard]] const std::set<QString>& tags() const;
		[[nodiscard]] std::set<QString>& mutableTags();
		[[nodiscard]] QStringList tagStringList() const;

		void setTags(std::set<QString> tags);

		[[nodiscard]] bool hasTag(const QString& tag) const;

		[[nodiscard]] E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat analogFormat);

		[[nodiscard]] int precision() const;
		void setPrecision(int value);

		[[nodiscard]] double lineWeight() const;
		void setLineWeight(double value);

		[[nodiscard]] double highLimit() const;
		void setHighLimit(double value);

		[[nodiscard]] double lowLimit() const;
		void setLowLimit(double value);

		[[nodiscard]] double viewHighLimit(E::TrendScaleType scaleType) const;
		void setViewHighLimit(E::TrendScaleType scaleType, double value);

		[[nodiscard]] double viewLowLimit(E::TrendScaleType scaleType) const;
		void setViewLowLimit(E::TrendScaleType scaleType, double value);

		[[nodiscard]] TrendColor color() const;
		void setColor(const TrendColor& value);

		// Temporary variables properties
		//
		[[nodiscard]] int tempSignalIndex() const;
		void setTempSignalIndex(int value);

		[[nodiscard]] const QRectF& tempDrawRect() const;
		void setTempDrawRect(const QRectF& value);

		// Data
		//
	private:
		QString m_signalId;				// CustomSignalID
		QString m_appSignalId;			// AppSignalID, starts from # for app data
		QString m_caption;
		QString m_equipmentId;

		ArchiveServer m_archiveServer;	// This field is set if data was aquired from the archive

		E::SignalType m_type = E::SignalType::Analog;
		QString m_unit;

		std::set<QString> m_tags;

		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		int m_precision = 0;

		double m_lineWeight = 0;		// 0 is cosmetic pen

		double m_highLimit = 1.0;
		double m_lowLimit = 0;

		std::map<E::TrendScaleType, TrendViewLimits> m_viewLimits; // Current view limits for signals for different scales

		TrendColor m_color = 0xFF000000;	// Black color

		// Temporary variables used in drawing
		//
	private:
		int m_tempSignalIndex = -1;	// Signal index, separate for disrctes and analogs, filled in getting signal list in TrendSignalSet::analogSignals/discreteSignals
		QRectF m_tempDrawRect;		// Draw signal area
	};

}

Q_DECLARE_METATYPE(TrendLib::TrendSignalParam)

#endif // TRENDSIGNAL_H
