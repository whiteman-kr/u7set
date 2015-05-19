#pragma once

#include "PosLineImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT VideoItemLine : public PosLineImpl
	{
		Q_OBJECT

		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<VideoItem>::DerivedType<VideoItemLine>;
#endif

	private:
		VideoItemLine(void);
	public:
		explicit VideoItemLine(SchemeUnit unit);
		virtual ~VideoItemLine(void);

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
