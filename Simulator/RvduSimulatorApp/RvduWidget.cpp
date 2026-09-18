#include <QPainter>
#include <QMouseEvent>
#include "RvduWidget.h"

#include <RvduSimulatorLib.h>
#include <Rvdu/keycodes.h>

RvduWidget::RvduWidget(unsigned int* pHdmiBuffer, int width, int height, QWidget* parent) :
	QWidget(parent),
	m_pHdmiBuffer(pHdmiBuffer),
	m_width(width),
	m_height(height)
{ 
}

RvduWidget::~RvduWidget() 
{
}

void RvduWidget::setRunningMode(bool running) 
{ 
	m_isRunning = running; 
}

void RvduWidget::setErrorMode(bool error, const QString& errorMsg) 
{ 
	m_isError = error; 
	m_errorMsg = errorMsg;
}

void RvduWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	if (m_isError == true) 
	{
		painter.drawText(0, 0, m_width, m_height, Qt::AlignCenter, m_errorMsg);
	}
	else
	{
		if (m_isRunning == true)
		{
			rnLibVideoBufferLock();

			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
			
			QImage image(reinterpret_cast<const uchar*>(m_pHdmiBuffer),
						 m_width,
						 m_height,
						 m_width * sizeof(unsigned int),
						 QImage::Format_RGB32);

			QRect widgetRect = this->rect();

			QRect targetRect = QStyle::alignedRect(Qt::LeftToRight,
												   Qt::AlignCenter,
												   image.size().scaled(widgetRect.size(), Qt::KeepAspectRatio),
												   widgetRect);

			painter.drawImage(targetRect, image);

			rnLibVideoBufferUnlock();
		}
	}
}

void RvduWidget::mousePressEvent(QMouseEvent* event)
{
	if (m_isRunning == true)
	{
		if (m_width == 0 || m_height == 0) 
		{
			Q_ASSERT(false);
			return;
		}

		QSize sourceSize(m_width, m_height);
		QRect widgetRect = this->rect();

		QRect targetRect =
			QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, sourceSize.scaled(widgetRect.size(), Qt::KeepAspectRatio), widgetRect);

		int x = event->pos().x();
		int y = event->pos().y();

		if (targetRect.contains(x, y) == false) 
		{
			return;
		}

		double kx = (double)targetRect.width() / m_width;
		double ky = (double)targetRect.height() / m_height;

		if (kx == 0.0 || ky == 0.0)
		{
			Q_ASSERT(false);
			return;
		}

		int dx = targetRect.x();
		int dy = targetRect.y();

		rnLibClickEvent((x - dx) / kx, (y - dy) / ky);
	}
}

void RvduWidget::keyPressEvent(QKeyEvent* event)
{ 
	if (m_isRunning == true)
	{
		unsigned int code = 0xffff;
		quint32 key = event->key();

		switch (key)
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			code = RK_RETURN;
			break;
		case Qt::Key_Escape:
			code = RK_ESCAPE;
			break;
		case Qt::Key_Delete:
			code = RK_DELETE;
			break;

		case Qt::Key_0:
			code = RK_NUMPAD0;
			break;
		case Qt::Key_1:
			code = RK_NUMPAD1;
			break;
		case Qt::Key_2:
			code = RK_NUMPAD2;
			break;
		case Qt::Key_3:
			code = RK_NUMPAD3;
			break;
		case Qt::Key_4:
			code = RK_NUMPAD4;
			break;
		case Qt::Key_5:
			code = RK_NUMPAD5;
			break;
		case Qt::Key_6:
			code = RK_NUMPAD6;
			break;
		case Qt::Key_7:
			code = RK_NUMPAD7;
			break;
		case Qt::Key_8:
			code = RK_NUMPAD8;
			break;
		case Qt::Key_9:
			code = RK_NUMPAD9;
			break;

		case Qt::Key_Comma:
			code = RK_DECIMAL;
			break;
		case Qt::Key_Minus:
			code = RK_SUBTRACT;
			break;
		case Qt::Key_Plus:
			code = RK_ADD;
			break;

		case Qt::Key_Up:
			code = RK_UP;
			break;
		case Qt::Key_Down:
			code = RK_DOWN;
			break;
		case Qt::Key_Left:
			code = RK_LEFT;
			break;
		case Qt::Key_Right:
			code = RK_RIGHT;
			break;
		case Qt::Key_Tab:
			code = RK_TAB;
			break;
		case Qt::Key_PageUp:
			code = RK_PRIOR;
			break;
		case Qt::Key_PageDown:
			code = RK_NEXT;
			break;
		case Qt::Key_Backspace:
			code = RK_BACKSPACE;
			break;
		case 'E':
			code = RK_E;
			break;
		case Qt::Key_QuoteLeft:
			code = RK_TILDA;
			break;
		default:
			break;
		}

		if (code != 0xffff)
		{
			rnLibKeyPressEvent(code);
		}
	}
}