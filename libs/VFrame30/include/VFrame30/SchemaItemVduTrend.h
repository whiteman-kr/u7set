#pragma once

#include <VFrame30/FontParam.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/PosRectImpl.h>
#include <VFrame30/SchemaItemVdu.h>

namespace Proto
{
	class SchemaItemVduTrendSignal;
	class SchemaItemVduTrendDuration;
}

namespace VFrame30
{
	class SchemaItemVduTrendSignalParam : public PropertyObject
	{
		Q_OBJECT

	public:
		SchemaItemVduTrendSignalParam();
		SchemaItemVduTrendSignalParam(const SchemaItemVduTrendSignalParam& src);

	private:
		void init();

	public:
		void save(Proto::SchemaItemVduTrendSignal* message) const;
		void load(const Proto::SchemaItemVduTrendSignal& message);

	public:
		QString appSignalId() const;
		void setAppSignalId(const QString& value);

		QString validityAppSignalId() const;
		void setValidityAppSignalId(const QString& value);

		int precision() const;
		void setPrecision(int value);

		E::DisplayValueFormat valueFormat() const;
		void setValueFormat(E::DisplayValueFormat value);

		double lowViewLimit() const;
		void setLowViewLimit(double value);

		double highViewLimit() const;
		void setHighViewLimit(double value);

		QColor color() const;
		void setColor(const QColor& value);

		int lineWeight() const;
		void setLineWeight(int value);

	private:
		QString m_appSignalId;
		QString m_validityAppSignalId;

		int m_precision = 0; // Number of digits after the decimal point
		E::DisplayValueFormat m_valueFormat = E::DisplayValueFormat::Auto;

		double m_lowViewLimit = 0.0;
		double m_highViewLimit = 100.0;

		QColor m_color = Qt::black;
		uint16_t m_lineWeight = 1;
	};

	class SchemaItemVduTrend : public PosRectImpl,
							   public IMatsSchemaItemAssociations,
							   public SchemaItemVduVisitable<SchemaItemVduTrend>
	{
		Q_OBJECT

	public:
		SchemaItemVduTrend(void);
		explicit SchemaItemVduTrend(SchemaUnit units);
		virtual ~SchemaItemVduTrend(void) = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

	private:
		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override;
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		QString durationsSecondsStr() const;
		void setDurationsSecondsStr(QString value);
		std::vector<uint32_t> durationsSeconds() const;

		int columnCount() const;

		E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		// Indent properties
		// Gets and sets indents in native units (mm/in/pixels).
		//
		int indentLeft() const;
		void setIndentLeft(int value);

		int indentRight() const;
		void setIndentRight(int value);

		int indentTop() const;
		void setIndentTop(int value);

		int indentBottom() const;
		void setIndentBottom(int value);

		// Appearance properties
		//
		QColor lineColor() const;
		void setLineColor(const QColor& value);

		QColor backColor() const;
		void setBackColor(const QColor& value);

		QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

		bool showSignalIds() const;
		void setShowSignalIds(bool value);

		bool showSignalCaptions() const;
		void setShowSignalCaptions(bool value);

		bool showSignalScales() const;
		void setShowSignalScales(bool value);

		bool showTimeLabels() const;
		void setShowTimeLabels(bool value);

		bool showDateLabels() const;
		void setShowDateLabels(bool value);

		bool use24hTimeFormat() const;
		void setUse24hTimeFormat(bool value);

		PropertyVector<SchemaItemVduTrendSignalParam> signalParams() const;
		void setSignalParams(const PropertyVector<SchemaItemVduTrendSignalParam>& value);

		DECLARE_FONT_PROPERTIES(Font)

	private:
		static const size_t MaxExtraDurations = 16;
		QString m_durationsSecs = "600";  // Durations, multiple can be set using separator

		E::TrendViewMode m_viewMode = E::TrendViewMode::Separated;
		E::TrendScaleType m_scaleType = E::TrendScaleType::Linear;

		static const size_t MaxSignalCount = 16;
		PropertyVector<SchemaItemVduTrendSignalParam> m_signalParams;

		int m_indentLeft{-1};
		int m_indentRight{-1};
		int m_indentTop{-1};
		int m_indentBottom{-1};

		QColor m_lineColor = {qRgb(0x80, 0x80, 0x80)};
		QColor m_backColor = {qRgb(0xE0, 0xE0, 0xE0)};
		QColor m_backColor1st = {qRgb(0xEA, 0xEA, 0xEA)};
		QColor m_backColor2nd = {qRgb(0xF8, 0xF8, 0xF8)};

		bool m_showSignalIds = true;
		bool m_showSignalCaptions = true;
		bool m_showSignalScales = true;
		bool m_showTimeLabels = true;
		bool m_showDateLabels = true;
		bool m_use24hTimeFormat = true;

		FontParam m_font;
	};
} // namespace VFrame30
