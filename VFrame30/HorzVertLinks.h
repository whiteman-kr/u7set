#pragma once

#include <limits>

namespace VFrame30
{
	struct SchemaPoint;

	// Используется для сохранения декомпозированных Link'ов, в BuildFblConnectionMap
	//
	class VFRAME30LIBSHARED_EXPORT CHorzVertLinks
	{
	public:
		void AddLinks(const std::list<SchemaPoint>& pointList, const QUuid& schemaItemGuid);

		// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
		//
		bool IsPointOnLink(SchemaPoint pt, const QUuid& schemaItemGuid);

		// Определить, лежит ли чточка на одном из отрезков vertlinks или на концах horzlinks
		//
		bool IsPinOnLink(SchemaPoint pt, const QUuid& schemaItemGuid);

		// Вернуть UUid элементов под точкой
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
				// Fake link are squized to the same Pos1 and Pos2,
				// make it a bit wider to fit point
				//
				return  val >= std::min(Pos1, Pos2) - 0.000001 &&
						val <= std::max(Pos1, Pos2) + 0.000001;

				//return val >= Pos1 && val <= Pos2;		// This will not work for fake points when Pos1 == Pos2
			}

			bool IsValOnEndPoints(double val)
			{
				return std::abs(val - Pos1) <= 0.000001 || std::abs(val - Pos2)  <= 0.000001;
			}
		};

		class LessClassFunctor
		{
		public:
			bool operator () (double v1, double v2) const
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


