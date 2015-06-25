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

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const override;

		// Properties and Data
	public:
		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

	private:
		double m_weight;					// ������� �����, �������� � ������ ��� ������ � ����������� �� UnitDocPt
		QRgb m_lineColor;
	};
}
