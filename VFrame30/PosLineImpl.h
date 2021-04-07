#pragma once

#include "SchemaItem.h"
#include "../lib/CUtils.h"
#include "Settings.h"

namespace VFrame30
{
	// ��������� ��� SchemaItem ������� ������ ���������� � ���� ������������ �����,
	// �������� ���� � ������ ���� � ������ � ����������� �� Unit
	//
	class IPosLine
	{
	public:
		virtual double startXDocPt() const = 0;
		virtual void setStartXDocPt(double value) = 0;

		virtual double startYDocPt() const = 0;
		virtual void setStartYDocPt(double value) = 0;

		virtual double endXDocPt() const = 0;
		virtual void setEndXDocPt(double value) = 0;

		virtual double endYDocPt() const = 0;
		virtual void setEndYDocPt(double value) = 0;
	};


	/*! \class PosLineImpl
		\brief PosLineImpl
	*/
	class PosLineImpl : public SchemaItem, public IPosLine
	{
		Q_OBJECT

		/// \brief Top position
		Q_PROPERTY(double Top READ top WRITE setTop)

		/// \brief Left position
		Q_PROPERTY(double Left READ left WRITE setLeft)

		/// \brief Width
		Q_PROPERTY(double Width READ width WRITE setWidth)

		/// \brief Height
		Q_PROPERTY(double Height READ height WRITE setHeight)

	protected:
		PosLineImpl(void);
		virtual ~PosLineImpl(void) = default;

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

		virtual void SetWidthInDocPt(double widthInDocPt) override;
		virtual void SetHeightInDocPt(double heightInDocPt) override;

		// Draw Functions
		//
	public:
		// ��������� �������� ��� ��� �������� ���������
		//
		virtual void drawOutline(CDrawParam* drawParam) const override;

		// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
		//
		virtual void drawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;

		virtual void drawCompareAction(CDrawParam* drawParam, QColor color) const override;

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

		// IPosLine implementation
		//
	private:
		double m_startXDocPt;
		double m_startYDocPt;
		double m_endXDocPt;
		double m_endYDocPt;

		// IPosLine implementation
		//
	public:
		virtual double startXDocPt() const override;
		virtual void setStartXDocPt(double value)  override;

		virtual double startYDocPt() const  override;
		virtual void setStartYDocPt(double value) override;

		virtual double endXDocPt() const  override;
		virtual void setEndXDocPt(double value)  override;

		virtual double endYDocPt() const  override;
		virtual void setEndYDocPt(double value)  override;

		// ISchemaItemPropertiesPos implementation
		//
	public:
		virtual double left() const  override;
		virtual void setLeft(double value)  override;

		virtual double top() const override;
		virtual void setTop(double value)  override;

		virtual double width() const override;
		virtual void setWidth(double value)  override;

		virtual double height() const override;
		virtual void setHeight(double value) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<SchemaPoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemaPoint>& points) override;
	};
}


