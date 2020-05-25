#ifndef DRAGDROP_H
#define DRAGDROP_H

#include <QMouseEvent>
#include <QDrag>
#include "../lib/AppSignalManager.h"

class DragDropHelper
{
public:
	DragDropHelper();

	void onMousePress(QMouseEvent* event, AppSignalParam appSignalParam);
	void onMouseMove(QMouseEvent* event, QObject* dragSource);

private:
	AppSignalParam m_appSignalParam;
	QPoint m_dragStartPosition;

};

#endif // DRAGDROP_H
