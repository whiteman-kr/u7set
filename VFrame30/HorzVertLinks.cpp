#include "Stable.h"
#include "HorzVertLinks.h"

namespace VFrame30
{
	// -- »спользуетс€ дл€ сохранени€ декомпозированных Link'ов, в BuildFblConnectionMap
	//
	void CHorzVertLinks::AddLinks(const std::list<VideoItemPoint>& pointList, const QUuid& VideoItemGuid)
	{
		// разложить кривую на отдельные отрезки и занести их в horzlinks и vertlinks
		//
		for (auto linkpoint = pointList.begin(); linkpoint != pointList.end(); ++linkpoint)
		{
			if (linkpoint == pointList.begin())
				continue;

			auto prevpoint = linkpoint;
			--prevpoint;

			VideoItemPoint pt1 = *prevpoint;
			VideoItemPoint pt2 = *linkpoint;

//			pt1.X = CUtils::Round(pt1.X, 5);
//			pt1.Y = CUtils::Round(pt1.Y, 5);

//			pt2.X = CUtils::Round(pt2.X, 5);
//			pt2.Y = CUtils::Round(pt2.Y, 5);

			if (std::abs(pt1.X - pt2.X) < 0.000001)	// is it verical line?
			{
				LINKS l(std::min(pt1.Y, pt2.Y), std::max(pt1.Y, pt2.Y), VideoItemGuid);
				vertlinks.insert(std::make_pair(pt1.X, l));
				continue;
			}

			if (std::abs(pt1.Y - pt2.Y) < 0.000001)	// is it horizontal line?
			{
				LINKS l(std::min(pt1.X, pt2.X), std::max(pt1.X, pt2.X), VideoItemGuid);
				horzlinks.insert(std::make_pair(pt1.Y, l));
				continue;
			}

			assert(false);	// line is neither vertiacal nor horizontal?
		}

		return;
	}

	// ќпределить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
	//
	bool CHorzVertLinks::IsPointOnLink(VideoItemPoint pt, const QUuid& VideoItemGuid)
	{
		// есть ли така€ точка в вертикальных отрезках?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				VideoItemGuid != vertline->second.VideoItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}

			++ vertline;
		}

		// есть ли така€ точка в горизонтальных отрезках?
		//

		// ѕока здесь не ищем, если расскооментировать, то пины обычных элементов будут цепл€тьс€ за √ќ–»«ќЌ“јЋ№Ќџ≈
		// линии, что некрасиво при продлении пина дальше соед. линии,
		// [_]--x-0---- примерно такой рисунок получитс€
		//

		auto horzline = horzlinks.find(pt.Y);
		while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
		{
			if (horzline->second.IsValInRange(pt.X) == true &&
				VideoItemGuid != horzline->second.VideoItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}

			++ horzline;
		}

		return false;
	}


}
