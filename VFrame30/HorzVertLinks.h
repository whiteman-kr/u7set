#pragma once

#include <limits>
#include "SchemeItem.h"

namespace VFrame30
{
	// ������������ ��� ���������� ����������������� Link'��, � BuildFblConnectionMap
	//
	class VFRAME30LIBSHARED_EXPORT CHorzVertLinks
	{
	public:
		void AddLinks(const std::list<SchemaPoint>& pointList, const QUuid& schemeItemGuid);

		// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
		//
		bool IsPointOnLink(SchemaPoint pt, const QUuid& schemeItemGuid);

		// ����������, ����� �� ������ �� ����� �� �������� vertlinks ��� �� ������ horzlinks
		//
		bool IsPinOnLink(SchemaPoint pt, const QUuid& schemeItemGuid);

		// ������� UUid ��������� ��� ������
		//
		std::list<QUuid> getSchemeItemsUnderPoint(SchemaPoint pt, QUuid schemeItemGuid);

	public:
		struct LINKS
		{
			LINKS(double p1, double p2, const QUuid& g)
			{
				Pos1 = p1;
				Pos2 = p2;
				SchemeItemGuid = g;
			}

			double Pos1;
			double Pos2;
			QUuid SchemeItemGuid;

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


