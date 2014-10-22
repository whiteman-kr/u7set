#pragma once

#include "VideoItem.h"
#include "Settings.h"

class QPen;

namespace VFrame30
{
	class CDrawParam;

	// ��������� ��� VideoItem ������� ������ ���������� � ���� ��������������, ���������� ������ ���� ���������������.
	// �������� ���� � ������ ���� � ������ � ����������� �� Unit
	//
	class IVideoItemPosRect
	{
	public:
		virtual double leftDocPt() const = 0;
		virtual void setLeftDocPt(double value) = 0;

		virtual double topDocPt() const = 0;
		virtual void setTopDocPt(double value) = 0;

		virtual double widthDocPt() const = 0;
		virtual void setWidthDocPt(double value) = 0;

		virtual double heightDocPt() const = 0;
		virtual void setHeightDocPt(double value) = 0;
	};


	// ���������� ������� ������������ ��� ��������� �������������� ����
	//
	class VFRAME30LIBSHARED_EXPORT PosRectImpl : public VideoItem, public IVideoItemPosRect
	{
		Q_OBJECT

	protected:
		PosRectImpl(void);
		virtual ~PosRectImpl(void);

	private:
		void Init(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Action Functions
		//
	public:
		virtual void MoveItem(double horzOffsetDocPt, double vertOffsetDocPt) override;

		virtual double GetWidthInDocPt() const override;
		virtual double GetHeightInDocPt() const override;

		virtual void SetWidthInDocPt(double val) override;
		virtual void SetHeightInDocPt(double val) override;

		// Draw Functions
		//
	public:
		// ��������� �������� ��� ��� �������� ���������
		//
		virtual void DrawOutline(CDrawParam* drawParam) const override;

		// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
		//
		virtual void DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;

		// Determine and Calculation Functions
		//
	public:
		// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
		// ���������� � ������ �������������� ������ � ������ ��� ��������
		// 
		virtual bool IsIntersectRect(double x, double y, double width, double height) const override;

		// Get VideoItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const override;

		// ���������� ����������� IVideoItemPosRect
		//
	private:
		double m_leftDocPt;
		double m_topDocPt;
		double m_widthDocPt;
		double m_heightDocPt;

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> selectionPen;
		mutable std::shared_ptr<QPen> outlinePen;

	public:
		virtual double leftDocPt() const override;
		virtual void setLeftDocPt(double value) override;

		virtual double topDocPt() const override;
		virtual void setTopDocPt(double value) override;

		virtual double widthDocPt() const override;
		virtual void setWidthDocPt(double value) override;

		virtual double heightDocPt() const override;
		virtual void setHeightDocPt(double value) override;

		// ���������� ����������� IVideoItemPropertiesPos
		//
	public:
		virtual double left() const override;
		virtual void setLeft(double value) override;

		virtual double top() const override;
		virtual void setTop(double value) override;

		virtual double width() const override;
		virtual void setWidth(double value) override;

		virtual double height() const override;
		virtual void setHeight(double value) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<VideoItemPoint> getPointList() const override;
		virtual void setPointList(const std::vector<VideoItemPoint>& points) override;
	};
}


