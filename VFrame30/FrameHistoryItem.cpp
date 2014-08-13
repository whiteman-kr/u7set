#include "Stable.h"
#include "FrameHistoryItem.h"

namespace VFrame30
{

	FrameHistoryItem::FrameHistoryItem() :
		m_zoom(100.0),
		m_horzScrollValue(0),
		m_vertScrollValue(0)
	{
	}

	FrameHistoryItem::FrameHistoryItem(const QString& oldStrID, const QString& newStrID, double zoom, int horzScrollValue, int vertScrollValue) :
		m_oldStrID(oldStrID),
		m_newStrID(newStrID), 
		m_zoom(zoom),
		m_horzScrollValue(horzScrollValue),
		m_vertScrollValue(vertScrollValue)
	{
	}

	FrameHistoryItem::~FrameHistoryItem()
	{
	}

	void FrameHistoryItem::setFrameParams(double zoom, int horzScrollValue, int vertScrollValue)
	{
		setZoom(zoom);
		setHorzScrollValue(horzScrollValue);
		setVertScrollValue(vertScrollValue);
	}

	const QString& FrameHistoryItem::oldStrID() const
	{
		return m_oldStrID;
	}
	
	void FrameHistoryItem::setOldStrID(const QString& strID)
	{
		m_oldStrID = strID;
	}

	const QString& FrameHistoryItem::newStrID() const
	{
		return m_newStrID;
	}
	
	void FrameHistoryItem::setNewStrID(const QString& strID)
	{
		m_newStrID = strID;
	}

	double FrameHistoryItem::zoom() const
	{
		return m_zoom;
	}

	void FrameHistoryItem::setZoom(double value)
	{
		m_zoom = value;
	}

	int FrameHistoryItem::horzScrollValue() const
	{
		return m_horzScrollValue;
	}

	void FrameHistoryItem::setHorzScrollValue(int value)
	{
		m_horzScrollValue = value;
	}

	int FrameHistoryItem::vertScrollValue() const
	{
		return m_vertScrollValue;
	}

	void FrameHistoryItem::setVertScrollValue(int value)
	{
		m_vertScrollValue = value;
	}

}
