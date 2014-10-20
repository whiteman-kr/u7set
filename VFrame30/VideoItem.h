#pragma once

#include "../include/ProtoSerialization.h"
#include "FontParam.h"
#include "DrawParam.h"
#include "DebugInstCounter.h"
#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class Scheme;
	class SchemeLayer;
}


namespace VFrame30
{
	struct VFRAME30LIBSHARED_EXPORT VideoItemPoint
	{
		double X;
		double Y;

		// �������
		//
		VideoItemPoint()
		{
			this->X = 0;
			this->Y = 0;
		}

		explicit VideoItemPoint(const Proto::VideoItemPoint& vip)
		{
			LoadData(vip);
		}

		explicit VideoItemPoint(QPointF point)
		{
			this->X = point.x();
			this->Y = point.y();
		}

		VideoItemPoint(double x, double y)
		{
			this->X = x;
			this->Y = y;
		}

		bool operator == (const VideoItemPoint& pt) const
		{
			return std::abs(pt.X - X) < 0.00001 && std::abs(pt.Y - Y) < 0.00001;
		}
		bool operator < (const VideoItemPoint& pt) const
		{
			if (operator==(pt) == true)
				return false;

			if (Y < pt.Y)
				return true;

			if (Y > pt.Y)
				return false;

			if (X < pt.X)
				return true;

			return false;
		}

		operator QPointF()
		{
			return QPointF(X, Y);
		}

		bool SaveData(Proto::VideoItemPoint* vip) const
		{
			vip->set_x(X);
			vip->set_y(Y);
			return true;
		}
		bool LoadData(const Proto::VideoItemPoint& vip)
		{
			this->X = vip.x();
			this->Y = vip.y();
			return true;
		}
	};

	// ��������� ��� SchemeItem ������� �������� ����� ��� �������� ��������� (ISchemePosRect, ISchemePosLine, ...) �
	// �������������, ��� ����������� � ��������� �������. ��������! ������� ��������� ���������� � �������� ��, �����, �����.
	// ��������! ��� �������� ������ ������������ ��� ��������� � ���������� ����� ���������!
	//
	class IVideoItemPropertiesPos
	{
	public:
		virtual double left() const = 0;
		virtual void setLeft(double value) = 0;

		virtual double top() const = 0;
		virtual void setTop(double value) = 0;

		virtual double width() const = 0;
		virtual void setWidth(double value) = 0;

		virtual double height() const = 0;
		virtual void setHeight(double value) = 0;
	};

	// ��������� IPointList, ��� ���������� � �������������� ������ �����.
	class IPointList
	{
	public:
		virtual std::vector<VideoItemPoint> getPointList() const = 0;
		virtual void setPointList(const std::vector<VideoItemPoint>& points) = 0;
	};


	class VFRAME30LIBSHARED_EXPORT VideoItem :
		public QObject, 
		public IVideoItemPropertiesPos, 
		public IPointList,
		public Proto::ObjectSerialization<VideoItem>,
		public DebugInstCounter<VideoItem>
	{
		Q_OBJECT

		Q_PROPERTY(bool AcceptClick READ acceptClick  WRITE setAcceptClick)
		Q_PROPERTY(QString ClickScript READ clickScript  WRITE setClickScript)

	protected:
		VideoItem();

	public:
		virtual ~VideoItem();

		// Serialization
		//
		friend Proto::ObjectSerialization<VideoItem>;	// ��� ������ CreateObject �� Proto::ObjectSerialization

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// ������������ ������� ������ ��� ������������, �.�. ��� �������� ������� �� ��������� �� ����������������,
		// � ������ �����������
		static VideoItem* CreateObject(const Proto::Envelope& message);

		// Action Functions
		//
	public:
		virtual void MoveItem(double horzOffsetDocPt, double vertOffsetDocPt);

		virtual double GetWidthInDocPt() const;
		virtual double GetHeightInDocPt() const;

		virtual void SetWidthInDocPt(double widthInDocPt);
		virtual void SetHeightInDocPt(double heightInDocPt);
		
		// Draw Functions
		//
	public:
		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* pDrawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const;

		// ��������� �������� ��� ��� �������� ���������
		//
		virtual void DrawOutline(CDrawParam* pDrawParam) const;
		static void DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<VideoItem>>& items);

		// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
		//
		virtual void DrawSelection(CDrawParam* pDrawParam, bool drawSizeBar) const;
		static void DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<VideoItem>>& items, bool drawSizeBar);

		// Determine and Calculation Functions
		//
	public:	
		// �����������, ������ �� ����� � �������, x � y � ������ ��� � ��������
		// 
		virtual bool IsIntersectPoint(double x, double y) const;

		// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
		// ���������� � ������ �������������� ������ � ������ ��� ��������
		// 
		virtual bool IsIntersectRect(double x, double y, double width, double height) const;

		// IVideoItemPropertiesPos interface implementation
		//
	public:
		virtual double left() const override;
		virtual void setLeft(double) override;

		virtual double top() const override;
		virtual void setTop(double) override;

		virtual double width() const override;
		virtual void setWidth(double) override;

		virtual double height() const override;
		virtual void setHeight(double) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<VideoItemPoint> getPointList() const override;
		virtual void setPointList(const std::vector<VideoItemPoint>& points) override;

		// Properties and Data
		//
	public:
		bool IsStatic() const;
		bool IsDynamic() const;

		virtual bool IsFblItem() const;

		bool IsLocked() const;
		void setLocked(bool locked);

		const QUuid& guid() const;
		void setGuid(const QUuid& guid);

		// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)
		//
		SchemeUnit itemUnit() const;
		void setItemUnit(SchemeUnit value);

		bool acceptClick() const;
		void setAcceptClick(bool value);

		const QString& clickScript() const;
		void setClickScript(const QString& value);

		// Get VideoItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const;		

		// Data
		//
	protected:
		bool m_static;
		bool m_locked;
		QUuid m_guid;
		SchemeUnit m_itemUnit;		// ������� ���������, � ������� �������� ���������� (����� ���� ������ ����� ��� �����)

		bool m_acceptClick;			// The VideoItem accept mouse Left button click and runs script
		QString m_clickScript;		// Qt script on mouse left button click
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::VideoItem> VideoItemFactory;
#endif
}


