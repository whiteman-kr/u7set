#pragma once
#include <QPointF>

namespace Proto
{
	class SchemaPoint;
}

namespace VFrame30
{
	struct SchemaPoint
	{
		double X;
		double Y;

		SchemaPoint();
		SchemaPoint(const Proto::SchemaPoint& vip);
		SchemaPoint(const QPointF& point);
		SchemaPoint(double x, double y);

		bool operator == (const QPointF& pt) const;
		bool operator == (const SchemaPoint& pt) const;
		bool operator < (const SchemaPoint& pt) const;

		operator QPointF() const;

		bool SaveData(Proto::SchemaPoint* vip) const;
		bool LoadData(const Proto::SchemaPoint& vip);
	};

}
