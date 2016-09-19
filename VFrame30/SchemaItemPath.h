#pragma once

#include "PosConnectionImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemPath : public PosConnectionImpl
	{
		Q_OBJECT

	public:
		SchemaItemPath(void);
		explicit SchemaItemPath(SchemaUnit unit);
		virtual ~SchemaItemPath(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const override;

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
