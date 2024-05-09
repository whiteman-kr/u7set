#pragma once

#include <QList>
#include <QObject>
#include <QPoint>

class QMouseEvent;

namespace SchemaClientLib
{
	class DragDropHelper
	{
	public:
		void onMousePress(QMouseEvent* event, QList<AppSignalParam> appSignalParams);
		void onMouseMove(QMouseEvent* event, QObject* dragSource);

	private:
		QList<AppSignalParam> m_appSignalParams;
		QPoint m_dragStartPosition;
	};
} // namespace SchemaClientLib
