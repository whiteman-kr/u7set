#ifndef DRAGDROP_H
#define DRAGDROP_H

#include <QMouseEvent>
#include <QDrag>
#include "../lib/AppSignalManager.h"

class DragDropHelper
{
public:
	DragDropHelper();

	void onMousePress(QMouseEvent* event, QList<AppSignalParam> appSignalParams);
	void onMouseMove(QMouseEvent* event, QObject* dragSource);

private:
	QList<AppSignalParam> m_appSignalParams;
	QPoint m_dragStartPosition;

};

#endif // DRAGDROP_H
