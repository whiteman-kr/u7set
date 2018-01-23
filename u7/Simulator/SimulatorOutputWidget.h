#ifndef SIMULATOROUTPUTWIDGET_H
#define SIMULATOROUTPUTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QMutex>

class SimulatorOutputWidget : public QTextEdit
{
	Q_OBJECT
public:
	explicit SimulatorOutputWidget(QWidget* parent = nullptr);

public:
	static void simulatorOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void keyPressEvent(QKeyEvent*) override;

private:
	static QtMessageHandler simOriginalMessageHandler;

	static QMutex m_mutex;
	static QStringList m_data;
};

#endif // SIMULATOROUTPUTWIDGET_H
