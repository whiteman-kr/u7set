#pragma once

#include "PosRectImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FblItemRect : public CPosRectImpl, public FblItem
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<FblItemRect>;
#endif

	protected:
		FblItemRect(void);
		FblItemRect(SchemeUnit itemunit);
	public:
		virtual ~FblItemRect(void);
		
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
		virtual void Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const override;

		// ��������� ���������� �����
		//
		virtual void SetConnectionsPos() override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const override;

		///<summary> 
		/// ���������� ��������� �����, ��� �������������� Fbl ��������
		///</summary>
		VideoItemPoint CalcPointPos(const QRectF& fblItemRect, const CFblConnectionPoint& connection, int pinCount, int index) const;

		// Properties and Data
		//
	public:
		virtual bool IsFblItem() const override;

		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

		QRgb fillColor() const;
		void setFillColor(QRgb color);

		QRgb textColor() const;
		void setTextColor(QRgb color);

		DECLARE_FONT_PROPERTIES(Font);
		
	protected:
		double m_weight;					// ������� �����, �������� � ������ ��� ������ � ����������� �� UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QRgb m_textColor;
		FontParam m_font;
	};
}


