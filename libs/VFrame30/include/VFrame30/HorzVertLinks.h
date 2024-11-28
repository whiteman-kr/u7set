#pragma once

#include <VFrame30/SchemaPoint.h>

namespace VFrame30
{
	// Используется для сохранения декомпозированных Link'ов, в BuildFblConnectionMap
	//
	class CHorzVertLinks
	{
	public:
		void clear()
		{
			horzlinks.clear();
			vertlinks.clear();
		}

		// -- Используется для сохранения декомпозированных Link'ов, в BuildFblConnectionMap
		//
		template <typename It>
		void AddLinks(It begin, It end, const QUuid& schemaItemGuid, size_t reserve /*= 4*/)
		{
			vertlinks.reserve(vertlinks.size() + reserve);
			horzlinks.reserve(horzlinks.size() + reserve);

			// разложить кривую на отдельные отрезки и занести их в horzlinks и vertlinks
			//
			for (auto linkpoint = begin; linkpoint != end; ++linkpoint)
			{
				if (linkpoint == begin)
					continue;

				auto prevpoint = linkpoint;
				--prevpoint;

				SchemaPoint pt1 = *prevpoint;
				SchemaPoint pt2 = *linkpoint;

				if (std::abs(pt1.X - pt2.X) < 0.000001)	// is it verical line?
				{
					LINKS l(std::min(pt1.Y, pt2.Y), std::max(pt1.Y, pt2.Y), schemaItemGuid);
					vertlinks.push_back({pt1.X, l});
					continue;
				}

				if (std::abs(pt1.Y - pt2.Y) < 0.000001)	// is it horizontal line?
				{
					LINKS l(std::min(pt1.X, pt2.X), std::max(pt1.X, pt2.X), schemaItemGuid);
					horzlinks.push_back({pt1.Y, l});
					continue;
				}

				assert(false);	// line is neither vertiacal nor horizontal?
			}

			return;
		}

		// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
		//
		bool IsPointOnLink(SchemaPoint pt, const QUuid& schemaItemGuid) const;

		// Определить, лежит ли чточка на одном из отрезков vertlinks или на концах horzlinks
		//
		bool IsPinOnLink(SchemaPoint pt, const QUuid& schemaItemGuid) const;

		// Вернуть UUid элементов под точкой
		//
		std::list<QUuid> getSchemaItemsUnderPoint(SchemaPoint pt, QUuid schemaItemGuid);

	public:
		struct LINKS
		{
			LINKS(double p1, double p2, const QUuid& g) :
				Pos1(p1),
				Pos2(p2),
				SchemaItemGuid(g)
			{
			}

			double Pos1;
			double Pos2;
			QUuid SchemaItemGuid;

			bool IsValInRange(double val) const
			{
				// Fake link are squized to the same Pos1 and Pos2,
				// make it a bit wider to fit point
				//
				return  val >= std::min(Pos1, Pos2) - 0.000001 &&
						val <= std::max(Pos1, Pos2) + 0.000001;

				//return val >= Pos1 && val <= Pos2;		// This will not work for fake points when Pos1 == Pos2
			}

			bool IsValOnEndPoints(double val) const
			{
				return std::abs(val - Pos1) <= 0.000001 || std::abs(val - Pos2)  <= 0.000001;
			}
		};

		//class LessClassFunctor
		//{
		//public:
		//	bool operator () (double v1, double v2) const
		//	{
		//		if (std::abs(v1 - v2) <= 0.000001)
		//		{
		//			return false;
		//		}
		//		else
		//		{
		//			return v1 < v2;
		//		}
		//	}
		//};

	public:
		std::vector<std::pair<double, LINKS>> horzlinks;
		std::vector<std::pair<double, LINKS>> vertlinks;
		
		//std::multimap<double, LINKS, LessClassFunctor> horzlinks;
		//std::multimap<double, LINKS, LessClassFunctor> vertlinks;
	};
}


