#include "SimulatorOutputWidget.h"
#include <QMenu>
#include <QAction>

QtMessageHandler SimulatorOutputWidget::simOriginalMessageHandler = 0;
QMutex SimulatorOutputWidget::m_mutex;
QStringList SimulatorOutputWidget::m_data;


SimulatorOutputWidget::SimulatorOutputWidget(QWidget* parent) :
	QTextEdit(parent)
{
	if (simOriginalMessageHandler == nullptr)
	{
		simOriginalMessageHandler = qInstallMessageHandler(simulatorOutputHandler);
	}

	// Output windows
	//
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setAutoFormatting(QTextEdit::AutoNone);
	document()->setUndoRedoEnabled(false);

	startTimer(25, Qt::PreciseTimer);

	return;
}

void SimulatorOutputWidget::simulatorOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if (QString(context.category) == QLatin1String("u7.sim"))
	{
		QString color;
		switch (type)
		{
		case QtMsgType::QtDebugMsg:
			color = "black";
			break;
		case QtMsgType::QtWarningMsg:
			color = "#F87217";
			break;
		case QtMsgType::QtCriticalMsg:
			color = "red";
			break;
		case QtMsgType::QtFatalMsg:
			color = "red";
			break;
		case QtMsgType::QtInfoMsg:
			color = "black";
			break;
		}

		QString time = QTime::currentTime().toString(QLatin1String("hh:mm:ss.zzz"));

		QString html = QString("<font face=\"Courier\" size=\"4\" color=#808080>%1 </font>"
							   "<font face=\"Courier\" size=\"4\" color=%2>%3</font>")
					   .arg(time)
					   .arg(color)
					   .arg(msg);

		QMutexLocker l(&m_mutex);
		m_data.push_back(html);

		if (m_data.size() > 1000)
		{
			m_data.pop_front();
		}
	}
	else
	{
		simOriginalMessageHandler(type, context, msg);
	}

	return;
}

void SimulatorOutputWidget::timerEvent(QTimerEvent*)
{
	QStringList data;
	{
		QMutexLocker l(&m_mutex);
		m_data.swap(data);
	}

	QString outputMessagesBuffer;
	outputMessagesBuffer.reserve(128000);

	for (int i = 0; i < data.size(); i++)
	{
		const QString& str = data[i];
		outputMessagesBuffer.append(str);

		if (i != data.size() - 1)
		{
			outputMessagesBuffer += QLatin1String("<br>");
		}
	}

	if (outputMessagesBuffer.isEmpty() == false)
	{
		this->append(outputMessagesBuffer);
	}

	return;
}

void SimulatorOutputWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu* menu = createStandardContextMenu();
	menu->addSeparator();

	QAction* clearAction = menu->addAction(tr("Clear"));

	QAction* selectedAction = menu->exec(event->globalPos());

	if (selectedAction == clearAction)
	{
		this->clear();
	}

	delete menu;
	return;
}

void SimulatorOutputWidget::keyPressEvent(QKeyEvent* e)
{
	if (e->matches(QKeySequence::Delete) == true)
	{
		QTextCursor cursor = this->textCursor();
		cursor.removeSelectedText();
		this->setTextCursor(cursor);

		e->accept();
		return;
	}

	if (e->matches(QKeySequence::SelectAll) == true)
	{
		this->selectAll();

		e->accept();
		return;
	}

	if (e->matches(QKeySequence::Cut) == true)
	{
		this->copy();		// this->cut() does not work

		QTextCursor cursor = this->textCursor();
		cursor.removeSelectedText();
		this->setTextCursor(cursor);

		e->accept();
		return;
	}

	QTextEdit::keyPressEvent(e);
	return;
}

