#pragma once

#include "SchemeItem.h"
#include "Settings.h"

namespace VFrame30
{
	// ��������� ��� VideoItem ������� ������ ���������� � ���� ������������ �����,
	// �������� ���� � ������ ���� � ������ � ����������� �� Unit
	//
	class IPosConnection
	{
	public:
		virtual const std::list<SchemePoint>& GetPointList() const = 0;
		virtual void SetPointList(const std::list<SchemePoint>& points) = 0;

		virtual void AddPoint(double x, double y) = 0;
		virtual void RemoveSamePoints() = 0;
		virtual void DeleteAllPoints() = 0;
		virtual void DeleteLastPoint() = 0;

		// ������ � Extension �������, ������� ��������� � ��������� ��� �������� ��������, DrawOutline
		//
		virtual const std::list<SchemePoint>& GetExtensionPoints() const = 0;
		virtual void SetExtensionPoints(const std::list<SchemePoint>& extPoints) = 0;
		virtual void AddExtensionPoint(double x, double y) = 0;
		virtual void DeleteAllExtensionPoints() = 0;
		virtual void DeleteLastExtensionPoint() = 0;
	};

	class VFRAME30LIBSHARED_EXPORT PosConnectionImpl : public SchemeItem, public IPosConnection
	{
		Q_OBJECT

	protected:
		PosConnectionImpl(void);
		virtual ~PosConnectionImpl(void);

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

		virtual void snapToGrid(double gridSize) override;

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

		// ���������� ����������� IVideoItemPosLine
		//
	private:
		std::list<SchemePoint> points;
		std::list<SchemePoint> extPoints;	// �����, ������� ������������ ��� DrawOutline, �� ���������������

	public:
		virtual const std::list<SchemePoint>& GetPointList() const override;
		virtual void SetPointList(const std::list<SchemePoint>& points) override;
		virtual void AddPoint(double x, double y) override;
		virtual void RemoveSamePoints() override;
		virtual void DeleteAllPoints() override;
		virtual void DeleteLastPoint() override;

		virtual const std::list<SchemePoint>& GetExtensionPoints() const override;
		virtual void SetExtensionPoints(const std::list<SchemePoint>& extPoints) override;
		virtual void AddExtensionPoint(double x, double y) override;
		virtual void DeleteAllExtensionPoints() override;
		virtual void DeleteLastExtensionPoint() override;

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
		virtual std::vector<SchemePoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemePoint>& points) override;
	};

}


