#ifndef HORZVERTLINKS_H
#define HORZVERTLINKS_H

#include "VideoItem.h"

namespace VFrame30
{
	// Используется для сохранения декомпозированных Link'ов, в BuildFblConnectionMap
	//
	class CHorzVertLinks
	{
	public:
		void AddLinks(const std::list<VideoItemPoint>& pointList, const QUuid& VideoItemGuid);

		// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
		//
		bool IsPointOnLink(VideoItemPoint pt, const QUuid& VideoItemGuid);

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
		};

		class LessClassFunctor
		{
		public:
			bool operator () (double v1, double v2)
			{
				v1 = CVFrameUtils::Round(v1, 5);
				v2 = CVFrameUtils::Round(v2, 5);
				return v1 < v2;
			}
		};

	public:
		std::multimap<double, LINKS, LessClassFunctor> horzlinks;
		std::multimap<double, LINKS, LessClassFunctor> vertlinks;
	};
}

#endif
