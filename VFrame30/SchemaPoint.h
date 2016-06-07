#pragma once
#include "../lib/ProtoSerialization.h"

namespace VFrame30
{
	struct VFRAME30LIBSHARED_EXPORT SchemaPoint
	{
		double X;
		double Y;

		SchemaPoint();
		explicit SchemaPoint(const Proto::SchemaPoint& vip);
		explicit SchemaPoint(QPointF point);
		SchemaPoint(double x, double y);

		bool operator == (const SchemaPoint& pt) const;
		bool operator < (const SchemaPoint& pt) const;

		operator QPointF() const;

		bool SaveData(Proto::SchemaPoint* vip) const;
		bool LoadData(const Proto::SchemaPoint& vip);
	};

}
