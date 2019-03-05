#pragma once

#include <QtGlobal>
#include <cassert>

enum class BinSearchResult
{
	RequireNextItem,
	NotFound,
	Found,

	SearchError
};

template <typename TYPE>
class BinSearch
{
public:
	BinSearch(TYPE soughtItem, qint64 itemsCount, TYPE leftItem, TYPE rightItem);

	BinSearchResult result() const { return m_result; }

	BinSearchResult checkNextItem(TYPE nextItem);

	qint64 nextItemIndex() const { assert(m_result == BinSearchResult::RequireNextItem && m_nextIndex != -1); return m_nextIndex; }

	qint64 foundIndex() const { assert(m_result == BinSearchResult::Found && m_foundIndex != -1); return m_foundIndex; }
	TYPE foundItem() const { assert(m_result == BinSearchResult::Found); return m_foundItem; }

	int iterations() const { return m_iteration; }

private:
	void calcRequiredIndex();

private:
	TYPE m_soughtItem;
	qint64 m_itemsCount = 0;

	TYPE m_leftItem;
	qint64 m_leftIndex = -1;

	TYPE m_rightItem;
	qint64 m_rightIndex = -1;

	TYPE m_foundItem;
	qint64 m_foundIndex = -1;

	qint64 m_nextIndex = -1;

	BinSearchResult m_result = BinSearchResult::NotFound;

	int m_iteration = 0;
};

template <typename TYPE>
BinSearch<TYPE>::BinSearch(TYPE soughtItem, qint64 itemsCount, TYPE leftItem, TYPE rightItem) :
	m_soughtItem(soughtItem),
	m_itemsCount(itemsCount),
	m_leftItem(leftItem),
	m_rightItem(rightItem)
{
	assert(itemsCount >= 0);

	m_iteration++;

	if (itemsCount == 0)
	{
		m_result = BinSearchResult::NotFound;
		return;
	}

	if (itemsCount == 1)
	{
		assert(leftItem == rightItem);
	}

	// check left bound
	//
	if (soughtItem <= leftItem)
	{
		m_foundItem = leftItem;
		m_foundIndex = 0;
		m_result = BinSearchResult::Found;
		return;
	}

	// check right bound
	//
	if (soughtItem > rightItem)
	{
		m_result = BinSearchResult::NotFound;
		return;
	}

	if (itemsCount == 2 && soughtItem <= rightItem)
	{
		assert(soughtItem > leftItem);

		m_foundItem = rightItem;
		m_foundIndex = 1;
		m_result = BinSearchResult::Found;
		return;
	}

	m_leftItem = leftItem;
	m_leftIndex = 0;

	m_rightItem = rightItem;
	m_rightIndex = itemsCount - 1;

	m_nextIndex = m_leftIndex + (m_rightIndex - m_leftIndex) / 2;

	m_result = BinSearchResult::RequireNextItem;
}

template <typename TYPE>
BinSearchResult BinSearch<TYPE>::checkNextItem(TYPE nextItem)
{
	m_iteration++;

	assert(m_leftIndex < m_rightIndex);

	if (nextItem < m_leftItem || nextItem > m_rightItem)
	{
		m_result = BinSearchResult::SearchError;
		return m_result;
	}

	if (nextItem >= m_soughtItem)
	{
		// move right bound
		//
		m_rightIndex = m_nextIndex;
		m_rightItem = nextItem;

		m_nextIndex = -1;
	}
	else
	{
		// nextItem < m_soughtItem

		// move left bound
		//
		m_leftIndex = m_nextIndex;
		m_leftItem = nextItem;

		m_nextIndex = -1;
	}

	qint64 distance = m_rightIndex - m_leftIndex;

	if (distance >= 2)
	{
		m_nextIndex = m_leftIndex + distance / 2;
		m_result = BinSearchResult::RequireNextItem;
	}
	else
	{
		// distance <= 2
		//
		if (distance != 1 ||
			m_leftItem >= m_soughtItem ||
			m_rightItem < m_soughtItem)
		{
			m_result = BinSearchResult::SearchError;
		}
		else
		{
			m_foundItem = m_rightItem;
			m_foundIndex = m_rightIndex;
			m_result = BinSearchResult::Found;
		}
	}

	return m_result;
}



