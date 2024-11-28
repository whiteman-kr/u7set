#include <VFrame30/HorzVertLinks.h>
#include <VFrame30/SchemaPoint.h>

namespace VFrame30
{
	// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
	//
	bool CHorzVertLinks::IsPointOnLink(SchemaPoint pt, const QUuid& schemaItemGuid) const
	{
		// есть ли такая точка в вертикальных отрезках?
		//
		for (const auto&[key, link] : vertlinks)
		{
			if (std::abs(key - pt.X) > 0.000001)
			{
				continue;
			}

			if (link.IsValInRange(pt.Y) == true &&
				schemaItemGuid != link.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}
		}

		//auto vertline = vertlinks.find(pt.X);
		//while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		//{
		//	if (vertline->second.IsValInRange(pt.Y) == true &&
		//		schemaItemGuid != vertline->second.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		return true;
		//	}

		//	++ vertline;
		//}

		// есть ли такая точка в горизонтальных отрезках?
		//

		// Пока здесь не ищем, если расскооментировать, то пины обычных элементов будут цепляться за ГОРИЗОНТАЛЬНЫЕ
		// линии, что некрасиво при продлении пина дальше соед. линии,
		// [_]--x-0---- примерно такой рисунок получится
		//

		for (const auto& [key, link] : horzlinks)
		{
			if (std::abs(key - pt.Y) > 0.000001)
			{
				continue;
			}

			if (link.IsValInRange(pt.X) == true &&
				schemaItemGuid != link.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}
		}

		//auto horzline = horzlinks.find(pt.Y);
		//while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
		//{
		//	if (horzline->second.IsValInRange(pt.X) == true &&
		//		schemaItemGuid != horzline->second.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		return true;
		//	}

		//	++ horzline;
		//}

		return false;
	}

	// Определить, лежит ли чточка на одном из отрезков horzlinks или vertlinks
	//
	bool CHorzVertLinks::IsPinOnLink(SchemaPoint pt, const QUuid& schemaItemGuid) const
	{
		// есть ли такая точка в вертикальных отрезках?
		//
		for (const auto& [key, link] : vertlinks)
		{
			if (std::abs(key - pt.X) > 0.000001)
			{
				continue;
			}

			if (link.IsValInRange(pt.Y) == true &&
				schemaItemGuid != link.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}
		}

		//auto vertline = vertlinks.find(pt.X);
		//while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		//{
		//	if (vertline->second.IsValInRange(pt.Y) == true &&
		//		schemaItemGuid != vertline->second.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		return true;
		//	}

		//	++ vertline;
		//}

		// есть ли такая точка в горизонтальных отрезках?
		//

		// Пока здесь не ищем, если расскооментировать, то пины обычных элементов будут цепляться за ГОРИЗОНТАЛЬНЫЕ
		// линии, что некрасиво при продлении пина дальше соед. линии,
		// [_]--x-0---- примерно такой рисунок получится
		//

		for (const auto& [key, link] : horzlinks)
		{
			if (std::abs(key - pt.Y) > 0.000001)
			{
				continue;
			}

			if (link.IsValOnEndPoints(pt.X) == true &&
				schemaItemGuid != link.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				return true;
			}
		}

		//auto horzline = horzlinks.find(pt.Y);
		//while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
		//{
		//	if (horzline->second.IsValOnEndPoints(pt.X) == true &&
		//		schemaItemGuid != horzline->second.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		return true;
		//	}

		//	++ horzline;
		//}

		return false;
	}

	std::list<QUuid> CHorzVertLinks::getSchemaItemsUnderPoint(SchemaPoint pt, QUuid schemaItemGuid)
	{
		std::list<QUuid> items;

		// есть ли такая точка в вертикальных отрезках?
		//
//		auto vertline = vertlinks.find(pt.X);

//		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
//		{
//			if (vertline->second.IsValInRange(pt.Y) == true &&
//				schemaItemGuid != vertline->second.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
//			{
//				items.push_back(vertline->second.SchemaItemGuid);
//			}

//			++ vertline;
//		}
		for (const auto& [key, link] : vertlinks)
		{
			if (std::abs(key - pt.X) > 0.000001)
			{
				continue;
			}

			if (link.IsValInRange(pt.Y) == true &&
				schemaItemGuid != link.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				items.push_back(link.SchemaItemGuid);
			}
		}

		//auto vertlineRange = vertlinks.equal_range(pt.X);

		//for (auto vertline = vertlineRange.first; vertline != vertlineRange.second; ++vertline)
		//{
		//	if (vertline->second.IsValInRange(pt.Y) == true &&
		//		schemaItemGuid != vertline->second.SchemaItemGuid)	// лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		items.push_back(vertline->second.SchemaItemGuid);
		//	}
		//}

		// есть ли такая точка в горизонтальных отрезках?
		//

		// Пока здесь не ищем, если расскооментировать, то пины обычных элементов будут цепляться за ГОРИЗОНТАЛЬНЫЕ
		// линии, что некрасиво при продлении пина дальше соед. линии,
		// [_]--x-0---- примерно такой рисунок получится
		//

//		auto horzline = horzlinks.find(pt.Y);

//		while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
//		{
//			if (horzline->second.IsValInRange(pt.X) == true &&
//				schemaItemGuid != horzline->second.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
//			{
//				items.push_back(horzline->second.SchemaItemGuid);
//			}

//			++ horzline;
//		}
		for (const auto& [key, link] : horzlinks)
		{
			if (std::abs(key - pt.Y) > 0.000001)
			{
				continue;
			}

			if (link.IsValInRange(pt.X) == true &&
				schemaItemGuid != link.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
			{
				items.push_back(link.SchemaItemGuid);
			}
		}

		//auto horzlineRange = horzlinks.equal_range(pt.Y);

		//for (auto horzline = horzlineRange.first; horzline != horzlineRange.second; ++horzline)
		//{
		//	if (horzline->second.IsValInRange(pt.X) == true &&
		//		schemaItemGuid != horzline->second.SchemaItemGuid) // лежит ли точка в диапазоне, и не пенедалжеит ли эта точка этому же эелементу
		//	{
		//		items.push_back(horzline->second.SchemaItemGuid);
		//	}
		//}

		return items;
	}

}
