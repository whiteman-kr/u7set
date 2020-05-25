#include "DragDropHelper.h"
#include "../Proto/serialization.pb.h"

DragDropHelper::DragDropHelper()
{

}

void DragDropHelper::onMousePress(QMouseEvent* event, AppSignalParam appSignalParam)
{
	if (event->button() != Qt::LeftButton)
	{
		return;
	}

	m_dragStartPosition = event->pos();
	m_appSignalParam = appSignalParam;
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
	::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
	m_appSignalParam.save(protoSignalMessage);

	QByteArray data;
	data.resize(protoSetMessage.ByteSize());

	protoSetMessage.SerializeToArray(data.data(), protoSetMessage.ByteSize());

	// --
	//
	if (data.isEmpty() == false)
	{
		QDrag* drag = new QDrag(dragSource);
		QMimeData* mimeData = new QMimeData;

		mimeData->setData(AppSignalParamMimeType::value, data);
		drag->setMimeData(mimeData);

		drag->exec(Qt::CopyAction);

		qDebug() << "Start drag for " << m_appSignalParam.appSignalId();
		qDebug() << "Drag and drop data buffer size " << data.size();
	}
}

