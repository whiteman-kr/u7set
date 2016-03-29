#pragma once

#include <limits>
#include "SchemaItem.h"

namespace VFrame30
{
	// ������������ ��� ���������� ����������������� Link'��, � BuildFblConnectionMap
	//
	class VFRAME30LIBSHARED_EXPORT CHorzVertLinks
	{
	public:
		void AddLinks(const std::list<SchemaPoint>& pointList, const QUuid& schemaItemGuid);

		// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
		//
		bool IsPointOnLink(SchemaPoint pt, const QUuid& schemaItemGuid);

		// ����������, ����� �� ������ �� ����� �� �������� vertlinks ��� �� ������ horzlinks
		//
		bool IsPinOnLink(SchemaPoint pt, const QUuid& schemaItemGuid);

		// ������� UUid ��������� ��� ������
		//
		std::list<QUuid> getSchemaItemsUnderPoint(SchemaPoint pt, QUuid schemaItemGuid);

	public:
		struct LINKS
		{
			LINKS(double p1, double p2, const QUuid& g)
			{
				Pos1 = p1;
				Pos2 = p2;
				SchemaItemGuid = g;
			}

			double Pos1;
			double Pos2;
			QUuid SchemaItemGuid;

			bool IsValInRange(double val)
			{
				return val >= Pos1 && val <= Pos2;
			}

			bool IsValOnEndPoints(double val)
			{
				return std::abs(val - Pos1) <= 0.000001 || std::abs(val - Pos2)  <= 0.000001;
			}
		};

		class LessClassFunctor
		{
		public:
			bool operator () (double v1, double v2)
			{
				if (std::abs(v1 - v2) <= 0.000001)
				{
					return false;
				}
				else
				{
					return v1 < v2;
				}
			}
		};

	public:
		std::multimap<double, LINKS, LessClassFunctor> horzlinks;
		std::multimap<double, LINKS, LessClassFunctor> vertlinks;
	};
}


