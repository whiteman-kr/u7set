#include "HorzVertLinks.h"
#include "SchemaPoint.h"

namespace VFrame30
{
	// -- ������������ ��� ���������� ����������������� Link'��, � BuildFblConnectionMap
	//
	void CHorzVertLinks::AddLinks(const std::list<SchemaPoint>& pointList, const QUuid& schemaItemGuid)
	{
		// ��������� ������ �� ��������� ������� � ������� �� � horzlinks � vertlinks
		//
		for (auto linkpoint = pointList.begin(); linkpoint != pointList.end(); ++linkpoint)
		{
			if (linkpoint == pointList.begin())
				continue;

			auto prevpoint = linkpoint;
			--prevpoint;

			SchemaPoint pt1 = *prevpoint;
			SchemaPoint pt2 = *linkpoint;

			if (std::abs(pt1.X - pt2.X) < 0.000001)	// is it verical line?
			{
				LINKS l(std::min(pt1.Y, pt2.Y), std::max(pt1.Y, pt2.Y), schemaItemGuid);
				vertlinks.insert(std::make_pair(pt1.X, l));
				continue;
			}

			if (std::abs(pt1.Y - pt2.Y) < 0.000001)	// is it horizontal line?
			{
				LINKS l(std::min(pt1.X, pt2.X), std::max(pt1.X, pt2.X), schemaItemGuid);
				horzlinks.insert(std::make_pair(pt1.Y, l));
				continue;
			}

			assert(false);	// line is neither vertiacal nor horizontal?
		}

		return;
	}

	// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
	//
	bool CHorzVertLinks::IsPointOnLink(SchemaPoint pt, const QUuid& schemaItemGuid)
	{
		// ���� �� ����� ����� � ������������ ��������?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemaItemGuid != vertline->second.SchemaItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ vertline;
		}

		// ���� �� ����� ����� � �������������� ��������?
		//

		// ���� ����� �� ����, ���� ������������������, �� ���� ������� ��������� ����� ��������� �� ��������������
		// �����, ��� ��������� ��� ��������� ���� ������ ����. �����,
		// [_]--x-0---- �������� ����� ������� ���������
		//

		auto horzline = horzlinks.find(pt.Y);
		while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
		{
			if (horzline->second.IsValInRange(pt.X) == true &&
				schemaItemGuid != horzline->second.SchemaItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ horzline;
		}

		return false;
	}

	// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
	//
	bool CHorzVertLinks::IsPinOnLink(SchemaPoint pt, const QUuid& schemaItemGuid)
	{
		// ���� �� ����� ����� � ������������ ��������?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemaItemGuid != vertline->second.SchemaItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ vertline;
		}

		// ���� �� ����� ����� � �������������� ��������?
		//

		// ���� ����� �� ����, ���� ������������������, �� ���� ������� ��������� ����� ��������� �� ��������������
		// �����, ��� ��������� ��� ��������� ���� ������ ����. �����,
		// [_]--x-0---- �������� ����� ������� ���������
		//

		auto horzline = horzlinks.find(pt.Y);
		while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
		{
			if (horzline->second.IsValOnEndPoints(pt.X) == true &&
				schemaItemGuid != horzline->second.SchemaItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ horzline;
		}

		return false;
	}

	std::list<QUuid> CHorzVertLinks::getSchemaItemsUnderPoint(SchemaPoint pt, QUuid schemaItemGuid)
	{
		std::list<QUuid> items;

		// ���� �� ����� ����� � ������������ ��������?
		//
//		auto vertline = vertlinks.find(pt.X);

//		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
//		{
//			if (vertline->second.IsValInRange(pt.Y) == true &&
//				schemaItemGuid != vertline->second.SchemaItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
//			{
//				items.push_back(vertline->second.SchemaItemGuid);
//			}

//			++ vertline;
//		}
		auto vertlineRange = vertlinks.equal_range(pt.X);

		for (auto vertline = vertlineRange.first; vertline != vertlineRange.second; ++vertline)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemaItemGuid != vertline->second.SchemaItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				items.push_back(vertline->second.SchemaItemGuid);
			}
		}

		// ���� �� ����� ����� � �������������� ��������?
		//

		// ���� ����� �� ����, ���� ������������������, �� ���� ������� ��������� ����� ��������� �� ��������������
		// �����, ��� ��������� ��� ��������� ���� ������ ����. �����,
		// [_]--x-0---- �������� ����� ������� ���������
		//

//		auto horzline = horzlinks.find(pt.Y);

//		while (horzline != horzlinks.end() && std::abs(horzline->first - pt.Y) < 0.000001)
//		{
//			if (horzline->second.IsValInRange(pt.X) == true &&
//				schemaItemGuid != horzline->second.SchemaItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
//			{
//				items.push_back(horzline->second.SchemaItemGuid);
//			}

//			++ horzline;
//		}
		auto horzlineRange = horzlinks.equal_range(pt.Y);

		for (auto horzline = horzlineRange.first; horzline != horzlineRange.second; ++horzline)
		{
			if (horzline->second.IsValInRange(pt.X) == true &&
				schemaItemGuid != horzline->second.SchemaItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				items.push_back(horzline->second.SchemaItemGuid);
			}
		}

		return items;
	}

}
