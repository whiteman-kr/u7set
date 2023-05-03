#ifndef APPLOGOUTPUTWIDGET_H
#define APPLOGOUTPUTWIDGET_H

class AppLogOutputWidget : public QTextEdit
{
	Q_OBJECT

public:
	explicit AppLogOutputWidget(QWidget* parent);

public:
	static void appLogOutputHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void contextMenuEvent(QContextMenuEvent* event) override;
	virtual void keyPressEvent(QKeyEvent*) override;

private:
	static QtMessageHandler appLogOriginalMessageHandler;

	static QMutex m_mutex;
	static QStringList m_data;
};

#endif // APPLOGOUTPUTWIDGET_H


