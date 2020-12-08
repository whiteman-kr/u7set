#pragma once

#include "SchemaItem.h"
#include "Settings.h"

namespace VFrame30
{
	// ��������� ��� SchemaItem ������� ������ ���������� � ���� ������������ �����,
	// �������� ���� � ������ ���� � ������ � ����������� �� Unit
	//
	class IPosConnection
	{
	public:
		virtual const std::list<SchemaPoint>& GetPointList() const = 0;
		virtual void SetPointList(const std::list<SchemaPoint>& points) = 0;

		virtual void AddPoint(double x, double y) = 0;
		virtual void RemoveSamePoints() = 0;
		virtual void DeleteAllPoints() = 0;
		virtual void DeleteLastPoint() = 0;
	};

	class VFRAME30LIBSHARED_EXPORT PosConnectionImpl : public SchemaItem, public IPosConnection
	{
		Q_OBJECT

	protected:
		PosConnectionImpl(void);
		virtual ~PosConnectionImpl(void) = default;

	private:
		void Init(void);

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Action Functions
		//
	public:
		virtual void moveItem(double horzOffsetDocPt, double vertOffsetDocPt) override;

		virtual void snapToGrid(double gridSize) override;

		virtual double GetWidthInDocPt() const override;
		virtual double GetHeightInDocPt() const override;

		virtual void SetWidthInDocPt(double val) override;
		virtual void SetHeightInDocPt(double val) override;

		// Othre funcs
		//
	public:
		virtual void dump() const override;

		// Draw Functions
		//
	public:
		// ��������� �������� ��� ��� �������� ���������
		//
		virtual void drawOutline(CDrawParam* drawParam) const override;

		// Draw item issue
		//
		virtual void drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const override;

		// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
		//
		virtual void drawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;

		virtual void drawCompareAction(CDrawParam* drawParam, QColor color) const override;

		// Draw comment "dim"
		//
		virtual void drawCommentDim(CDrawParam* drawParam) const;

		// Determine and Calculation Functions
		//
	public:
		// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
		// ���������� � ������ �������������� ������ � ������ ��� ��������
		// 
		virtual bool isIntersectRect(double x, double y, double width, double height) const override;

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt(const CDrawParam* drawParam) const override;

		// IPosLine
		//
	private:
		std::list<SchemaPoint> points;
		std::list<SchemaPoint> extPoints;	// �����, ������� ������������ ��� DrawOutline, �� ���������������

	public:
		virtual const std::list<SchemaPoint>& GetPointList() const override;
		virtual void SetPointList(const std::list<SchemaPoint>& points) override;
		virtual void AddPoint(double x, double y) override;
		virtual void RemoveSamePoints() override;
		virtual void DeleteAllPoints() override;
		virtual void DeleteLastPoint() override;

		// ���������� ����������� ISchemaItemPropertiesPos
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
		virtual std::vector<SchemaPoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemaPoint>& points) override;
	};

}


