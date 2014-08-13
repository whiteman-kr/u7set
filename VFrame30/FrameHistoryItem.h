#ifndef FRAMEHISTORYITEM_H
#define FRAMEHISTORYITEM_H

namespace VFrame30
{
	class FrameHistoryItem
	{
	public:
		FrameHistoryItem();
		FrameHistoryItem(const QString& oldStrID, const QString& newStrID, double zoom, int horzScrollValue, int vertScrollValue);
		~FrameHistoryItem();

	public:
		void setFrameParams(double zoom, int horzScrollValue, int vertScrollValue);

		const QString& oldStrID() const;
		void setOldStrID(const QString& strID);

		const QString& newStrID() const;
		void setNewStrID(const QString& strID);

		double zoom() const;
		void setZoom(double value);

		int horzScrollValue() const;
		void setHorzScrollValue(int value);

		int vertScrollValue() const;
		void setVertScrollValue(int value);

	private:
		QString m_oldStrID;
		QString m_newStrID;

		double m_zoom;
		int m_horzScrollValue;
		int m_vertScrollValue;
	};
}

#endif // FRAMEHISTORYITEM_H
