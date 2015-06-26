#pragma once

#include "PosConnectionImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemeItemPath : public PosConnectionImpl
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemPath>;
#endif

	private:
		SchemeItemPath(void);
	public:
		explicit SchemeItemPath(SchemeUnit unit);
		virtual ~SchemeItemPath(void);

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
		virtual void Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const override;

		// Properties and Data
	public:
		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
	};
}
