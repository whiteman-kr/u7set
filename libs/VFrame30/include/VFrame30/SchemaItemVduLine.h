#pragma once

#include <VFrame30/PosLineImpl.h>
#include <VFrame30/SchemaItemVdu.h>

namespace VFrame30
{

	class SchemaItemVduLine final : public PosLineImpl,
									public SchemaItemVduVisitable<SchemaItemVduLine>
	{
		Q_OBJECT

	public:
		SchemaItemVduLine(void);
		explicit SchemaItemVduLine(SchemaUnit unit);
		virtual ~SchemaItemVduLine(void) = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		// Item is drawn in 100% scale
		// Graphics must have screen coordinate system (0, 0 - left upper corner, down and right - positive pos)
		//
		virtual void draw(CDrawParam* drawParam) const override;

		// Properties and Data
		//
	public:
		int weight() const;
		void setWeight(int weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

	private:
		int m_weight;
		QColor m_lineColor;
	};
} // namespace VFrame30
