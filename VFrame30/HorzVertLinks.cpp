#include "Stable.h"
#include "HorzVertLinks.h"

namespace VFrame30
{
	// -- ������������ ��� ���������� ����������������� Link'��, � BuildFblConnectionMap
	//
	void CHorzVertLinks::AddLinks(const std::list<SchemaPoint>& pointList, const QUuid& schemeItemGuid)
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
				LINKS l(std::min(pt1.Y, pt2.Y), std::max(pt1.Y, pt2.Y), schemeItemGuid);
				vertlinks.insert(std::make_pair(pt1.X, l));
				continue;
			}

			if (std::abs(pt1.Y - pt2.Y) < 0.000001)	// is it horizontal line?
			{
				LINKS l(std::min(pt1.X, pt2.X), std::max(pt1.X, pt2.X), schemeItemGuid);
				horzlinks.insert(std::make_pair(pt1.Y, l));
				continue;
			}

			assert(false);	// line is neither vertiacal nor horizontal?
		}

		return;
	}

	// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
	//
	bool CHorzVertLinks::IsPointOnLink(SchemaPoint pt, const QUuid& schemeItemGuid)
	{
		// ���� �� ����� ����� � ������������ ��������?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemeItemGuid != vertline->second.SchemeItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
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
				schemeItemGuid != horzline->second.SchemeItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ horzline;
		}

		return false;
	}

	// ����������, ����� �� ������ �� ����� �� �������� horzlinks ��� vertlinks
	//
	bool CHorzVertLinks::IsPinOnLink(SchemaPoint pt, const QUuid& schemeItemGuid)
	{
		// ���� �� ����� ����� � ������������ ��������?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemeItemGuid != vertline->second.SchemeItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
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
				schemeItemGuid != horzline->second.SchemeItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				return true;
			}

			++ horzline;
		}

		return false;
	}

	std::list<QUuid> CHorzVertLinks::getSchemeItemsUnderPoint(SchemaPoint pt, QUuid schemeItemGuid)
	{
		std::list<QUuid> items;

		// ���� �� ����� ����� � ������������ ��������?
		//
		auto vertline = vertlinks.find(pt.X);
		while (vertline != vertlinks.end() && std::abs(vertline->first - pt.X) < 0.000001)
		{
			if (vertline->second.IsValInRange(pt.Y) == true &&
				schemeItemGuid != vertline->second.SchemeItemGuid)	// ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				items.push_back(vertline->second.SchemeItemGuid);
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
				schemeItemGuid != horzline->second.SchemeItemGuid) // ����� �� ����� � ���������, � �� ����������� �� ��� ����� ����� �� ���������
			{
				items.push_back(horzline->second.SchemeItemGuid);
			}

			++ horzline;
		}


		return items;
	}

}
