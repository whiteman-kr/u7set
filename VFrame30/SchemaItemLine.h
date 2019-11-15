#pragma once

#include "PosLineImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemLine : public PosLineImpl
	{
		Q_OBJECT

	public:
		SchemaItemLine(void);
		explicit SchemaItemLine(SchemaUnit unit);
		virtual ~SchemaItemLine(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const final;

		// Properties and Data
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QColor m_lineColor;
	};
}
