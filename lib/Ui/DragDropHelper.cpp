#include "DragDropHelper.h"
#include "../Proto/serialization.pb.h"

DragDropHelper::DragDropHelper()
{

}

void DragDropHelper::onMousePress(QMouseEvent* event, QList<AppSignalParam> appSignalParams)
{
	if (event->button() != Qt::LeftButton)
	{
		return;
	}

	m_dragStartPosition = event->pos();
	m_appSignalParams = appSignalParams;
}

void DragDropHelper::onMouseMove(QMouseEvent* event, QObject* dragSource)
{
	if (event->buttons().testFlag(Qt::LeftButton) == false)
	{
		return;
	}

	if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
	{
		return;
	}

	// Save signals to protobufer
	//
	::Proto::AppSignalSet protoSetMessage;

	for (auto appSignalParam : m_appSignalParams)
	{
		::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
		appSignalParam.save(protoSignalMessage);
	}

	QByteArray data;
	data.resize(static_cast<int>(protoSetMessage.ByteSizeLong()));

	protoSetMessage.SerializeToArray(data.data(), static_cast<int>(protoSetMessage.ByteSizeLong()));

	// --
	//
	if (data.isEmpty() == false)
	{
		QDrag* drag = new QDrag(dragSource);
		QMimeData* mimeData = new QMimeData;

		mimeData->setData(AppSignalParamMimeType::value, data);
		drag->setMimeData(mimeData);

		drag->exec(Qt::CopyAction);
	}
}

