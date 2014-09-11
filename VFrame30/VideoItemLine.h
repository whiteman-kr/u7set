#pragma once

#include "PosLineImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CVideoItemLine : public CPosLineImpl
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemLine>;
#endif

	private:
		CVideoItemLine(void);
	public:
		explicit CVideoItemLine(SchemeUnit unit);
		virtual ~CVideoItemLine(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(::Proto::Envelope* message) const override;
		virtual bool LoadData(const ::Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const override;

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
