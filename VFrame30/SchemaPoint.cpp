#include "SchemaPoint.h"

namespace VFrame30
{

	SchemaPoint::SchemaPoint() :
		X(0),
		Y(0)
	{
	}

	SchemaPoint::SchemaPoint(const Proto::SchemaPoint& vip)
	{
		LoadData(vip);
	}

	SchemaPoint::SchemaPoint(QPointF point) :
		X(point.x()),
		Y(point.y())
	{
	}

	SchemaPoint::SchemaPoint(double x, double y) :
		X(x),
		Y(y)
	{
	}

	bool SchemaPoint::operator== (const SchemaPoint& pt) const
	{
		return std::abs(pt.X - X) < 0.000001 && std::abs(pt.Y - Y) < 0.000001;
	}

	bool SchemaPoint::operator < (const SchemaPoint& pt) const
	{
		if (operator==(pt) == true)
		{
			return false;
		}

		if (std::abs(pt.Y - Y) < 0.000001)
		{
			return X < pt.X;
		}

		if (Y < pt.Y)
			return true;

		if (Y > pt.Y)
			return false;

		return false;
	}

	SchemaPoint::operator QPointF() const
	{
		return QPointF(X, Y);
	}

	bool SchemaPoint::SaveData(Proto::SchemaPoint* vip) const
	{
		vip->set_x(X);
		vip->set_y(Y);
		return true;
	}
	bool SchemaPoint::LoadData(const Proto::SchemaPoint& vip)
	{
		this->X = vip.x();
		this->Y = vip.y();
		return true;
	}


}
