#pragma once

#include <limits>
#include "VideoItem.h"

namespace VFrame30
{
	// Используется для сохранения декомпозированных Link'ов, в BuildFblConnectionMap
	//
	class VFRAME30LIBSHARED_EXPORT CHorzVertLinks
	{
	public:
		void AddLinks(const std::list<SchemePoint>& pointList, const QUuid& VideoItemGuid);

		// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
		//
		bool IsPointOnLink(SchemePoint pt, const QUuid& VideoItemGuid);

		// Определить, лежит ли чточка на одном из отрезков vertlinks или на концах horzlinks
		//
		bool IsPinOnLink(SchemePoint pt, const QUuid& VideoItemGuid);

		// Вернуть UUid элементов под точкой
		//
		std::list<QUuid> getVideoItemsUnderPoint(SchemePoint pt, QUuid VideoItemGuid);

	public:
		struct LINKS
		{
			LINKS(double p1, double p2, const QUuid& g)
			{
				Pos1 = p1;
				Pos2 = p2;
				VideoItemGuid = g;
			}

			double Pos1;
			double Pos2;
			QUuid VideoItemGuid;

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


